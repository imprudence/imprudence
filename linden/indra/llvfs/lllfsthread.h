/** 
 * @file lllfsthread.h
 * @brief LLLFSThread base class
 *
 * Copyright (c) 2000-2007, Linden Research, Inc.
 * 
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#ifndef LL_LLLFSTHREAD_H
#define LL_LLLFSTHREAD_H

#include <queue>
#include <string>
#include <map>
#include <set>

#include "llapr.h"

#include "llqueuedthread.h"

//============================================================================
// Threaded Local File System
//============================================================================

class LLLFSThread : public LLQueuedThread
{
	//------------------------------------------------------------------------
public:
	enum operation_t {
		FILE_READ,
		FILE_WRITE,
		FILE_RENAME,
		FILE_REMOVE
	};

	//------------------------------------------------------------------------
public:

	class Request : public QueuedRequest
	{
	protected:
		~Request() {}; // use deleteRequest()
		
	public:
		Request(handle_t handle, U32 priority, U32 flags,
				operation_t op, const LLString& filename,
				U8* buffer, S32 offset, S32 numbytes);

		S32 getBytes()
		{
			return mBytes;
		}
		S32 getBytesRead()
		{
			return mBytesRead;
		}
		S32 getOperation()
		{
			return mOperation;
		}
		U8* getBuffer()
		{
			return mBuffer;
		}
		const LLString& getFilename()
		{
			return mFileName;
		}
		
		/*virtual*/ void finishRequest();
		/*virtual*/ void deleteRequest();

		bool processIO();
		
	private:
		operation_t mOperation;
		
		LLString mFileName;
		
		U8* mBuffer;	// dest for reads, source for writes, new UUID for rename
		S32 mOffset;	// offset into file, -1 = append (WRITE only)
		S32 mBytes;		// bytes to read from file, -1 = all
		S32	mBytesRead;	// bytes read from file
	};

	//------------------------------------------------------------------------
public:
	LLLFSThread(bool threaded = TRUE, bool runalways = TRUE);
	~LLLFSThread();	

	// Return a Request handle
	handle_t read(const LLString& filename,
				  U8* buffer, S32 offset, S32 numbytes, U32 pri=PRIORITY_NORMAL, U32 flags = 0);
	handle_t write(const LLString& filename,
				   U8* buffer, S32 offset, S32 numbytes, U32 flags = 0);
	handle_t rename(const LLString& filename, const LLString& newname, U32 flags = 0);
	handle_t remove(const LLString& filename, U32 flags = 0);
	
	// Return number of bytes read
	S32 readImmediate(const LLString& filename,
					  U8* buffer, S32 offset, S32 numbytes);
	S32 writeImmediate(const LLString& filename,
					   U8* buffer, S32 offset, S32 numbytes);

	static void initClass(bool local_is_threaded = TRUE, bool run_always = TRUE); // Setup sLocal
	static S32 updateClass(U32 ms_elapsed);
	static void cleanupClass();		// Delete sLocal

protected:
	/*virtual*/ bool processRequest(QueuedRequest* req);

public:
	static LLLFSThread* sLocal;		// Default local file thread
};

//============================================================================


#endif // LL_LLLFSTHREAD_H
