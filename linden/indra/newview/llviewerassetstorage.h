/** 
 * @file llviewerassetstorage.h
 * @brief Class for loading asset data to/from an external source.
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
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

#ifndef LLVIEWERASSETSTORAGE_H
#define LLVIEWERASSETSTORAGE_H

#include "llassetstorage.h"
//#include "curl/curl.h"

class LLVFile;

class LLViewerAssetStorage : public LLAssetStorage
{
public:
	typedef void (*LLStoreAssetCallback)(const LLUUID &transaction_id, void *user_data, S32 status);

	LLViewerAssetStorage(LLMessageSystem *msg, LLXferManager *xfer,
				   LLVFS *vfs, const LLHost &upstream_host);

	LLViewerAssetStorage(LLMessageSystem *msg, LLXferManager *xfer,
				   LLVFS *vfs);

	virtual void storeAssetData(
		const LLTransactionID& tid,
		LLAssetType::EType atype,
		LLStoreAssetCallback callback,
		void* user_data,
		bool temp_file = false,
		bool is_priority = false,
		bool store_local = false);
	
	virtual void storeAssetData(
		const char* filename,
		const LLTransactionID& tid,
		LLAssetType::EType type,
		LLStoreAssetCallback callback,
		void* user_data,
		bool temp_file = false,
		bool is_priority = false);
};

#endif
