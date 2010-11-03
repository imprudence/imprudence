

#include "llviewerprecompiledheaders.h"

#include "hipporestrequest.h"

#ifndef CURL_STATICLIB
#define CURL_STATICLIB 1
#endif
#include <curl/curl.h>

#include <stdtypes.h>
#include <llbufferstream.h>
#include <llerror.h>
#include <llhttpclient.h>
#include <llurlrequest.h>
#include <llxmltree.h>


// ********************************************************************
 
 
class HippoRestComplete : public LLURLRequestComplete
{
	public:
		HippoRestComplete(HippoRestHandler *handler) :
			mHandler(handler),
			mStatus(499),
			mReason("Request completed w/o status")
		{
		}
 
		~HippoRestComplete()
		{
			delete mHandler;
		}
 
		// Called once for each header received, prior to httpStatus
		void header(const std::string& header, const std::string& value)
		{
			mHandler->addHeader(header, value);
		}

		// Always called on request completion, prior to complete
		void httpStatus(U32 status, const std::string& reason)
		{
			LLURLRequestComplete::httpStatus(status, reason);
			mStatus = status;
			mReason = reason;
		}

		void complete(const LLChannelDescriptors &channels, const buffer_ptr_t &buffer)
		{
			mHandler->handle(mStatus, mReason, channels, buffer);
		}

	private:
		HippoRestHandler *mHandler;
		int mStatus;
		std::string mReason;
};


// ********************************************************************


static std::string gEmptyString;

void HippoRestHandler::addHeader(const std::string &header, const std::string &content)
{
	mHeaders[header] = content;
}

bool HippoRestHandler::hasHeader(const std::string &header) const
{
	return (mHeaders.find(header) != mHeaders.end());
}

const std::string &HippoRestHandler::getHeader(const std::string &header) const
{
	std::map<std::string, std::string>::const_iterator it;
	it = mHeaders.find(header);
	if (it != mHeaders.end()) {
		return it->second;
	} else {
		return gEmptyString;
	}
}


// ********************************************************************


void HippoRestHandlerRaw::handle(int status, const std::string &reason,
								 const LLChannelDescriptors &channels,
								 const boost::shared_ptr<LLBufferArray> &body)
{
	if (status == 200) {
		std::string data;
		LLBufferArray *buffer = body.get();
		LLBufferArray::segment_iterator_t it, end = buffer->endSegment();
		for (it=buffer->beginSegment(); it!=end; ++it)
			if (it->isOnChannel(channels.in()))
				data.append((char*)it->data(), it->size());
		result(data);
	} else {
		llwarns << "Rest request error " << status << ": " << reason << llendl;
	}
}

void HippoRestHandlerXml::handle(int status, const std::string &reason,
								 const LLChannelDescriptors &channels,
								 const boost::shared_ptr<LLBufferArray> &body)
{
	if (status == 200) {
		LLXmlTree *tree = new LLXmlTree();
		bool success = tree->parseBufferStart();
		LLBufferArray *buffer = body.get();
		LLBufferArray::segment_iterator_t it, end = buffer->endSegment();
		for (it=buffer->beginSegment(); success && (it!=end); ++it)
			if (it->isOnChannel(channels.in()))
				success = success && tree->parseBuffer((char*)it->data(), it->size());
		success = success && tree->parseBufferFinalize();
		if (success) result(tree);
		delete tree;
	} else {
		llwarns << "Rest request error " << status << ": " << reason << llendl;
	}
}


// ********************************************************************


class BodyData : public LLIOPipe
{
	public:
		virtual ~BodyData() { }
		virtual const char *getContentMimeType() const = 0;
};

class BodyDataRaw : public BodyData
{
	public:
		explicit BodyDataRaw(const std::string &data) :
			mData(data)
		{
		}
		virtual ~BodyDataRaw() { }
 
		const char *getContentMimeType() const { return "application/octet-stream"; }
 
		EStatus process_impl(const LLChannelDescriptors &channels,
							 buffer_ptr_t &buffer, bool &eos,
							 LLSD &context, LLPumpIO *pump)
		{
			LLBufferStream ostream(channels, buffer.get());
			ostream.write(mData.data(), mData.size());
			eos = true;
			return STATUS_DONE;
		}

	private:
		std::string mData;
};

class BodyDataXml : public BodyData
{
	public:
		explicit BodyDataXml(const LLXmlTree *tree) :
			mTree(tree)
		{
		}

		virtual ~BodyDataXml()
		{
			if (mTree) delete mTree;
		}

		const char *getContentMimeType() const { return "application/xml"; }

		EStatus process_impl(const LLChannelDescriptors &channels,
							 buffer_ptr_t &buffer, bool &eos,
							 LLSD &context, LLPumpIO *pump)
		{
			std::string data;
			mTree->write(data);
			LLBufferStream ostream(channels, buffer.get());
			ostream.write(data.data(), data.size());
			eos = true;
			return STATUS_DONE;
		}

	private:
		const LLXmlTree *mTree;
};


// ********************************************************************


static void request(const std::string &url,
					LLURLRequest::ERequestAction method,
					BodyData *body,
					HippoRestHandler *handler, float timeout);


// static
void HippoRestRequest::get(const std::string &url,
						   HippoRestHandler *handler, float timeout)
{
	request(url, LLURLRequest::HTTP_GET, 0, handler, timeout);
}

// static
void HippoRestRequest::put(const std::string &url, const std::string &body,
						   HippoRestHandler *handler, float timeout)
{
	request(url, LLURLRequest::HTTP_PUT, new BodyDataRaw(body), handler, timeout);
}

// static
void HippoRestRequest::put(const std::string &url, const LLXmlTree *body,
						   HippoRestHandler *handler, float timeout)
{
	request(url, LLURLRequest::HTTP_PUT, new BodyDataXml(body), handler, timeout);
}

// static
void HippoRestRequest::post(const std::string &url, const std::string &body,
							HippoRestHandler *handler, float timeout)
{
	request(url, LLURLRequest::HTTP_POST, new BodyDataRaw(body), handler, timeout);
}

// static
void HippoRestRequest::post(const std::string &url, const LLXmlTree *body,
							HippoRestHandler *handler, float timeout)
{
	request(url, LLURLRequest::HTTP_POST, new BodyDataXml(body), handler, timeout);
}


// ********************************************************************


static void request(const std::string &url,
					LLURLRequest::ERequestAction method,
					BodyData *body,
					HippoRestHandler *handler, float timeout)
{
	if (!LLHTTPClient::hasPump())
	{
		// !!! responder->completed(U32_MAX, "No pump", LLSD());
		return;
	}
	LLPumpIO::chain_t chain;

	LLURLRequest *req = new LLURLRequest(method, url);
	req->checkRootCertificate(true);

	/*
	// Insert custom headers if the caller sent any
	if (headers.isMap())
	{
		LLSD::map_const_iterator iter = headers.beginMap();
		LLSD::map_const_iterator end  = headers.endMap();
 
		for (; iter != end; ++iter)
		{
			std::ostringstream header;
			//if the header is "Pragma" with no value
			//the caller intends to force libcurl to drop
			//the Pragma header it so gratuitously inserts
			//Before inserting the header, force libcurl
			//to not use the proxy (read: llurlrequest.cpp)
			static const std::string PRAGMA("Pragma");
			if ((iter->first == PRAGMA) && (iter->second.asString().empty()))
			{
				req->useProxy(false);
			}
			header << iter->first << ": " << iter->second.asString() ;
			lldebugs << "header = " << header.str() << llendl;
			req->addHeader(header.str().c_str());
		}
	}
	*/

	if ((method != LLURLRequest::HTTP_PUT) && (method != LLURLRequest::HTTP_POST)) {
		std::string accept = "Accept: ";
		accept += handler->getAcceptMimeType();
		req->addHeader(accept.c_str());
	}

	req->setCallback(new HippoRestComplete(handler));

	if ((method == LLURLRequest::HTTP_PUT) || (method == LLURLRequest::HTTP_POST)) {
		std::string content = "Content-Type: ";
		content += body->getContentMimeType();
		req->addHeader(content.c_str());
		chain.push_back(LLIOPipe::ptr_t(body));
	}

	chain.push_back(LLIOPipe::ptr_t(req));
	LLHTTPClient::getPump().addChain(chain, timeout);
}


// ********************************************************************


static size_t curlWrite(void *ptr, size_t size, size_t nmemb, void *userData)
{
	std::string *result = (std::string*)userData;
	size_t bytes = (size * nmemb);
	result->append((char*)ptr, bytes);
	return nmemb;
}


// static
int HippoRestRequest::getBlocking(const std::string &url, std::string *result)
{
	llinfos << "Requesting: " << url << llendl;

	char curlErrorBuffer[CURL_ERROR_SIZE];
	CURL* curlp = curl_easy_init();

	curl_easy_setopt(curlp, CURLOPT_NOSIGNAL, 1);	// don't use SIGALRM for timeouts
	curl_easy_setopt(curlp, CURLOPT_TIMEOUT, 5);	// seconds

	curl_easy_setopt(curlp, CURLOPT_WRITEFUNCTION, curlWrite);
	curl_easy_setopt(curlp, CURLOPT_WRITEDATA, result);
	curl_easy_setopt(curlp, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curlp, CURLOPT_ERRORBUFFER, curlErrorBuffer);
	curl_easy_setopt(curlp, CURLOPT_FAILONERROR, 1);

	*result = "";
	S32 curlSuccess = curl_easy_perform(curlp);
	S32 httpStatus = 499;
	curl_easy_getinfo(curlp, CURLINFO_RESPONSE_CODE, &httpStatus);

	if (curlSuccess != 0) {
		llwarns << "CURL ERROR (HTTP Status " << httpStatus << "): " << curlErrorBuffer << llendl;
	} else if (httpStatus != 200) {
		llwarns << "HTTP Error " << httpStatus << ", but no Curl error." << llendl;
	}
	
	curl_easy_cleanup(curlp);
	return httpStatus;
}

