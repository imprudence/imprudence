/** 
 * @file lltexturetable.h
 * @brief Table of texture names and IDs for viewer
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

// Table of texture information for the viewer UI. The Table is
// maintained in a sorted order at all times.

#ifndef LL_LLTEXTURETABLE_H
#define LL_LLTEXTURETABLE_H

#include "lluuid.h"
#include "linked_lists.h"

class LLTextureInfo
{
public:
	LLTextureInfo(const std::string& name, const LLUUID &uuid, const std::string& description);
	~LLTextureInfo();
	std::string	mName;
	LLUUID		mUUID;
	std::string	mDescription;
};

class LLTextureTable
{
public:
	LLTextureTable();
	~LLTextureTable();
	
	BOOL	add(const std::string& name, const LLUUID &uuid, 
				const std::string& description);				// copies memory, false if fails
	LLUUID	getUUID(const std::string& name);					// LLUUID::null if not found
	std::string getDesc(const std::string& name);
	std::string	getName(const LLUUID &id);
	void	deleteAll();
	//void	bubbleSort();

	static BOOL insertBefore(LLTextureInfo* a, LLTextureInfo* b);

public:
	LLLinkedList<LLTextureInfo>	mTextureInfoList;
};

extern LLTextureTable gTextureTable;

#endif
