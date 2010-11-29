/** 
 * @file lliosocket.cpp
 * @author Phoenix
 * @date 2005-07-31
 * @brief Sockets declarations for use with the io pipes
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "linden_common.h"
#include "lliosocket.h"

#include "llapr.h"

#include "llbuffer.h"
#include "llhost.h"
#include "llmemtype.h"
#include "llpumpio.h"
#include "llthread.h"

//
// constants
//

static const S32 LL_DEFAULT_LISTEN_BACKLOG = 10;
static const S32 LL_SEND_BUFFER_SIZE = 40000;
static const S32 LL_RECV_BUFFER_SIZE = 40000;
//static const U16 LL_PORT_DISCOVERY_RANGE_MIN = 13000;
//static const U16 LL_PORT_DISCOVERY_RANGE_MAX = 13050;

//
// local methods 
//

bool is_addr_in_use(apr_status_t status)
{
#if LL_WINDOWS
	return (WSAEADDRINUSE == APR_TO_OS_ERROR(status));
#else
	return (EADDRINUSE == APR_TO_OS_ERROR(status));
#endif
}

#if LL_LINUX
// Define this to see the actual file descriptors being tossed around.
//#define LL_DEBUG_SOCKET_FILE_DESCRIPTORS 1
#if LL_DEBUG_SOCKET_FILE_DESCRIPTORS
#include "apr_portable.h"
#endif
#endif


// Quick function 
void ll_debug_socket(const char* msg, apr_socket_t* apr_sock)
{
#if LL_DEBUG_SOCKET_FILE_DESCRIPTORS
	if(!apr_sock)
	{
		lldebugs << "Socket -- " << (msg?msg:"") << ": no socket." << llendl;
		return;
	}
	// *TODO: Why doesn't this work?
	//apr_os_sock_t os_sock;
	int os_sock;
	if(APR_SUCCESS == apr_os_sock_get(&os_sock, apr_sock))
	{
		lldebugs << "Socket -- " << (msg?msg:"") << " on fd " << os_sock
			<< " at " << apr_sock << llendl;
	}
	else
	{
		lldebugs << "Socket -- " << (msg?msg:"") << " no fd "
			<< " at " << apr_sock << llendl;
	}
#endif
}

///
/// LLSocket
///

// static
LLSocket::ptr_t LLSocket::create(EType type, U16 port)
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	apr_status_t status = APR_EGENERAL;
	LLSocket::ptr_t rv(new LLSocket);

	if(STREAM_TCP == type)
	{
		status = apr_socket_create(&rv->mSocket, APR_INET, SOCK_STREAM, APR_PROTO_TCP, rv->mPool());
	}
	else if(DATAGRAM_UDP == type)
	{
		status = apr_socket_create(&rv->mSocket, APR_INET, SOCK_DGRAM, APR_PROTO_UDP, rv->mPool());
	}
	else
	{
		rv.reset();
		return rv;
	}
	if(ll_apr_warn_status(status))
	{
		rv->mSocket = NULL;
		rv.reset();
		return rv;
	}
	if(port > 0)
	{
		apr_sockaddr_t* sa = NULL;
		status = apr_sockaddr_info_get(
			&sa,
			APR_ANYADDR,
			APR_UNSPEC,
			port,
			0,
			rv->mPool());
		if(ll_apr_warn_status(status))
		{
			rv.reset();
			return rv;
		}
		// This allows us to reuse the address on quick down/up. This
		// is unlikely to create problems.
		ll_apr_warn_status(apr_socket_opt_set(rv->mSocket, APR_SO_REUSEADDR, 1));
		status = apr_socket_bind(rv->mSocket, sa);
		if(ll_apr_warn_status(status))
		{
			rv.reset();
			return rv;
		}
		lldebugs << "Bound " << ((DATAGRAM_UDP == type) ? "udp" : "tcp")
				 << " socket to port: " << sa->port << llendl;
		if(STREAM_TCP == type)
		{
			// If it's a stream based socket, we need to tell the OS
			// to keep a queue of incoming connections for ACCEPT.
			lldebugs << "Setting listen state for socket." << llendl;
			status = apr_socket_listen(
				rv->mSocket,
				LL_DEFAULT_LISTEN_BACKLOG);
			if(ll_apr_warn_status(status))
			{
				rv.reset();
				return rv;
			}
		}
	}
	else
	{
		// we need to indicate that we have an ephemeral port if the
		// previous calls were successful. It will
		port = PORT_EPHEMERAL;
	}
	rv->mPort = port;
	rv->setOptions();
	return rv;
}

// static
LLSocket::ptr_t LLSocket::create(apr_status_t& status, LLSocket::ptr_t& listen_socket)
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	if (!listen_socket->getSocket())
	{
		status = APR_ENOSOCKET;
		return LLSocket::ptr_t();
	}
	LLSocket::ptr_t rv(new LLSocket);
	lldebugs << "accepting socket" << llendl;
	status = apr_socket_accept(&rv->mSocket, listen_socket->getSocket(), rv->mPool());
	if (status != APR_SUCCESS)
	{
		rv->mSocket = NULL;
		rv.reset();
		return rv;
	}
	rv->mPort = PORT_EPHEMERAL;
	rv->setOptions();
	return rv;
}

bool LLSocket::blockingConnect(const LLHost& host)
{
	if(!mSocket) return false;
	apr_sockaddr_t* sa = NULL;
	std::string ip_address;
	ip_address = host.getIPString();
	if(ll_apr_warn_status(apr_sockaddr_info_get(
		&sa,
		ip_address.c_str(),
		APR_UNSPEC,
		host.getPort(),
		0,
		mPool())))
	{
		return false;
	}
	apr_socket_timeout_set(mSocket, 1000);
	ll_debug_socket("Blocking connect", mSocket);
	if(ll_apr_warn_status(apr_socket_connect(mSocket, sa))) return false;
	setOptions();
	return true;
}

LLSocket::LLSocket() :
	mSocket(NULL),
	mPool(LLThread::tldata().mRootPool),
	mPort(PORT_INVALID)
{
}

LLSocket::~LLSocket()
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	// *FIX: clean up memory we are holding.
	if(mSocket)
	{
		ll_debug_socket("Destroying socket", mSocket);
		apr_socket_close(mSocket);
	}
}

void LLSocket::setOptions()
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	// set up the socket options
	ll_apr_warn_status(apr_socket_timeout_set(mSocket, 0));
	ll_apr_warn_status(apr_socket_opt_set(mSocket, APR_SO_SNDBUF, LL_SEND_BUFFER_SIZE));
	ll_apr_warn_status(apr_socket_opt_set(mSocket, APR_SO_RCVBUF, LL_RECV_BUFFER_SIZE));

}

///
/// LLIOSocketReader
///

LLIOSocketReader::LLIOSocketReader(LLSocket::ptr_t socket) :
	mSource(socket),
	mInitialized(false)
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
}

LLIOSocketReader::~LLIOSocketReader()
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	//lldebugs << "Destroying LLIOSocketReader" << llendl;
}

// virtual
LLIOPipe::EStatus LLIOSocketReader::process_impl(
	const LLChannelDescriptors& channels,
	buffer_ptr_t& buffer,
	bool& eos,
	LLSD& context,
	LLPumpIO* pump)
{
	PUMP_DEBUG;
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	if(!mSource) return STATUS_PRECONDITION_NOT_MET;
	if(!mInitialized)
	{
		PUMP_DEBUG;
		// Since the read will not block, it's ok to initialize and
		// attempt to read off the descriptor immediately.
		mInitialized = true;
		if(pump)
		{
			PUMP_DEBUG;
			lldebugs << "Initializing poll descriptor for LLIOSocketReader."
					 << llendl;
			apr_pollfd_t poll_fd;
			poll_fd.p = NULL;
			poll_fd.desc_type = APR_POLL_SOCKET;
			poll_fd.reqevents = APR_POLLIN;
			poll_fd.rtnevents = 0x0;
			poll_fd.desc.s = mSource->getSocket();
			poll_fd.client_data = NULL;
			pump->setConditional(this, &poll_fd);
		}
	}
	//if(!buffer)
	//{
	//	buffer = new LLBufferArray;
	//}
	PUMP_DEBUG;
	const apr_size_t READ_BUFFER_SIZE = 1024;
	char read_buf[READ_BUFFER_SIZE]; /*Flawfinder: ignore*/
	apr_size_t len;
	apr_status_t status = APR_SUCCESS;
	do
	{
		PUMP_DEBUG;
		len = READ_BUFFER_SIZE;
		status = apr_socket_recv(mSource->getSocket(), read_buf, &len);
		buffer->append(channels.out(), (U8*)read_buf, len);
	} while((APR_SUCCESS == status) && (READ_BUFFER_SIZE == len));
	lldebugs << "socket read status: " << status << llendl;
	LLIOPipe::EStatus rv = STATUS_OK;

	PUMP_DEBUG;
	// *FIX: Also need to check for broken pipe
	if(APR_STATUS_IS_EOF(status))
	{
		// *FIX: Should we shut down the socket read?
		if(pump)
		{
			pump->setConditional(this, NULL);
		}
		rv = STATUS_DONE;
		eos = true;
	}
	else if(APR_STATUS_IS_EAGAIN(status))
	{
/*Commented out by Aura 9-9-8 for DEV-19961.
		// everything is fine, but we can terminate this process pump.
	
		rv = STATUS_BREAK;
*/
	}
	else
	{
		if(ll_apr_warn_status(status))
		{
			rv = STATUS_ERROR;
		}
	}
	PUMP_DEBUG;
	return rv;
}

///
/// LLIOSocketWriter
///

LLIOSocketWriter::LLIOSocketWriter(LLSocket::ptr_t socket) :
	mDestination(socket),
	mLastWritten(NULL),
	mInitialized(false)
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
}

LLIOSocketWriter::~LLIOSocketWriter()
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	//lldebugs << "Destroying LLIOSocketWriter" << llendl;
}

// virtual
LLIOPipe::EStatus LLIOSocketWriter::process_impl(
	const LLChannelDescriptors& channels,
	buffer_ptr_t& buffer,
	bool& eos,
	LLSD& context,
	LLPumpIO* pump)
{
	PUMP_DEBUG;
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	if(!mDestination) return STATUS_PRECONDITION_NOT_MET;
	if(!mInitialized)
	{
		PUMP_DEBUG;
		// Since the write will not block, it's ok to initialize and
		// attempt to write immediately.
		mInitialized = true;
		if(pump)
		{
			PUMP_DEBUG;
			lldebugs << "Initializing poll descriptor for LLIOSocketWriter."
					 << llendl;
			apr_pollfd_t poll_fd;
			poll_fd.p = NULL;
			poll_fd.desc_type = APR_POLL_SOCKET;
			poll_fd.reqevents = APR_POLLOUT;
			poll_fd.rtnevents = 0x0;
			poll_fd.desc.s = mDestination->getSocket();
			poll_fd.client_data = NULL;
			pump->setConditional(this, &poll_fd);
		}
	}

	PUMP_DEBUG;
	// *FIX: Some sort of writev implementation would be much more
	// efficient - not only because writev() is better, but also
	// because we won't have to do as much work to find the start
	// address.
	LLBufferArray::segment_iterator_t it;
	LLBufferArray::segment_iterator_t end = buffer->endSegment();
	LLSegment segment;
	it = buffer->constructSegmentAfter(mLastWritten, segment);
	/*
	if(NULL == mLastWritten)
	{
		it = buffer->beginSegment();
		segment = (*it);
	}
	else
	{
		it = buffer->getSegment(mLastWritten);
		segment = (*it);
		S32 size = segment.size();
		U8* data = segment.data();
		if((data + size) == mLastWritten)
		{
			++it;
			segment = (*it);
		}
		else
		{
			// *FIX: check the math on this one
			segment = LLSegment(
				(*it).getChannelMask(),
				mLastWritten + 1,
				size - (mLastWritten - data));
		}
	}
	*/

	PUMP_DEBUG;
	apr_size_t len;
	bool done = false;
	apr_status_t status = APR_SUCCESS;
	while(it != end)
	{

		PUMP_DEBUG;
		if((*it).isOnChannel(channels.in()))
		{
			PUMP_DEBUG;
			len = (apr_size_t)segment.size();
			status = apr_socket_send(
				mDestination->getSocket(),
				(const char*)segment.data(),
				&len);
			// We sometimes get a 'non-blocking socket operation could not be 
			// completed immediately' error from apr_socket_send.  In this
			// case we break and the data will be sent the next time the chain
			// is pumped.
			if(APR_STATUS_IS_EAGAIN(status))
			{
				ll_apr_warn_status(status);
				break;
			}

			mLastWritten = segment.data() + len - 1;

			PUMP_DEBUG;
			if((S32)len < segment.size())
			{
				break;
			}
			
		}

		++it;
		if(it != end)
		{
			segment = (*it);
		}
		else
		{
			done = true;
		}

	}
	PUMP_DEBUG;
	if(done && eos)
	{
		return STATUS_DONE;
	}
	return STATUS_OK;
}


///
/// LLIOServerSocket
///

LLIOServerSocket::LLIOServerSocket(
	LLIOServerSocket::socket_t listener,
	factory_t factory) :
	mListenSocket(listener),
	mReactor(factory),
	mInitialized(false),
	mResponseTimeout(DEFAULT_CHAIN_EXPIRY_SECS)
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
}

LLIOServerSocket::~LLIOServerSocket()
{
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	//lldebugs << "Destroying LLIOServerSocket" << llendl;
}

void LLIOServerSocket::setResponseTimeout(F32 timeout_secs)
{
	mResponseTimeout = timeout_secs;
}

// virtual
LLIOPipe::EStatus LLIOServerSocket::process_impl(
	const LLChannelDescriptors& channels,
	buffer_ptr_t& buffer,
	bool& eos,
	LLSD& context,
	LLPumpIO* pump)
{
	PUMP_DEBUG;
	LLMemType m1(LLMemType::MTYPE_IO_TCP);
	if(!pump)
	{
		llwarns << "Need a pump for server socket." << llendl;
		return STATUS_ERROR;
	}
	if(!mInitialized)
	{
		PUMP_DEBUG;
		// This segment sets up the pump so that we do not call
		// process again until we have an incoming read, aka connect()
		// from a remote host.
		lldebugs << "Initializing poll descriptor for LLIOServerSocket."
				 << llendl;
		apr_pollfd_t poll_fd;
		poll_fd.p = NULL;
		poll_fd.desc_type = APR_POLL_SOCKET;
		poll_fd.reqevents = APR_POLLIN;
		poll_fd.rtnevents = 0x0;
		poll_fd.desc.s = mListenSocket->getSocket();
		poll_fd.client_data = NULL;
		pump->setConditional(this, &poll_fd);
		mInitialized = true;
		return STATUS_OK;
	}

	// we are initialized, and told to process, so we must have a
	// socket waiting for a connection.
	lldebugs << "accepting socket" << llendl;

	PUMP_DEBUG;
	apr_status_t status;
	LLSocket::ptr_t llsocket(LLSocket::create(status, mListenSocket));
	//EStatus rv = STATUS_ERROR;
	if(llsocket && status == APR_SUCCESS)
	{
		PUMP_DEBUG;

		apr_sockaddr_t* remote_addr;
		apr_socket_addr_get(&remote_addr, APR_REMOTE, llsocket->getSocket());
		
		char* remote_host_string;
		apr_sockaddr_ip_get(&remote_host_string, remote_addr);

		LLSD context;
		context["remote-host"] = remote_host_string;
		context["remote-port"] = remote_addr->port;

		LLPumpIO::chain_t chain;
		chain.push_back(LLIOPipe::ptr_t(new LLIOSocketReader(llsocket)));
		if(mReactor->build(chain, context))
		{
			chain.push_back(LLIOPipe::ptr_t(new LLIOSocketWriter(llsocket)));
			pump->addChain(chain, mResponseTimeout);
		}
		else
		{
			llwarns << "Unable to build reactor to socket." << llendl;
		}
	}
	else
	{
		char buf[256];
		llwarns << "Unable to accept linden socket: " << apr_strerror(status, buf, sizeof(buf)) << llendl;
	}

	PUMP_DEBUG;
	// This needs to always return success, lest it get removed from
	// the pump.
	return STATUS_OK;
}


#if 0
LLIODataSocket::LLIODataSocket(
	U16 suggested_port,
	U16 start_discovery_port) :
	mSocket(NULL)
{
	if(PORT_INVALID == suggested_port) return;
	if(ll_apr_warn_status(apr_socket_create(&mSocket, APR_INET, SOCK_DGRAM, APR_PROTO_UDP, pool))) return;
	apr_sockaddr_t* sa = NULL;
	if(ll_apr_warn_status(apr_sockaddr_info_get(&sa, APR_ANYADDR, APR_UNSPEC, suggested_port, 0, pool))) return;
	apr_status_t status = apr_socket_bind(mSocket, sa);
	if((start_discovery_port > 0) && is_addr_in_use(status))
	{
		const U16 MAX_ATTEMPT_PORTS = 50;
		for(U16 attempt_port = start_discovery_port;
			attempt_port < (start_discovery_port + MAX_ATTEMPT_PORTS);
			++attempt_port)
		{
			sa->port = attempt_port;
			sa->sa.sin.sin_port = htons(attempt_port);
			status = apr_socket_bind(mSocket, sa);
			if(APR_SUCCESS == status) break;
			if(is_addr_in_use(status)) continue;
			(void)ll_apr_warn_status(status);
		}
	}
	if(ll_apr_warn_status(status)) return;
	if(sa->port)
	{
		lldebugs << "Bound datagram socket to port: " << sa->port << llendl;
		mPort = sa->port;
	}
	else
	{
		mPort = LLIOSocket::PORT_EPHEMERAL;
	}

	// set up the socket options options
	ll_apr_warn_status(apr_socket_timeout_set(mSocket, 0));
	ll_apr_warn_status(apr_socket_opt_set(mSocket, APR_SO_SNDBUF, LL_SEND_BUFFER_SIZE));
	ll_apr_warn_status(apr_socket_opt_set(mSocket, APR_SO_RCVBUF, LL_RECV_BUFFER_SIZE));
}

LLIODataSocket::~LLIODataSocket()
{
}


#endif
