/** 
 * @file lltexturetable.cpp
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

#include "llviewerprecompiledheaders.h"

#include <string.h>

#include "lltexturetable.h"
#include "llstring.h"

LLTextureTable gTextureTable;


LLTextureInfo::LLTextureInfo(const std::string& name, const LLUUID &uuid, const std::string& description)
{
	S32 length = name.size();
	if (length > 0)
	{
		mName = name;
	}
	else
	{
		char *temp = new char[UUID_STR_LENGTH];
		uuid.toString(temp);
		mName = temp;
	}
	LLString::toLower(mName);
	
	mUUID = uuid;
	mDescription = description;
}

LLTextureInfo::~LLTextureInfo()
{
}

LLTextureTable::LLTextureTable()
{
	mTextureInfoList.setInsertBefore(&LLTextureTable::insertBefore);
}

LLTextureTable::~LLTextureTable()
{
	mTextureInfoList.deleteAllData();
}

// Comparison function for sorting the list.
// static
BOOL LLTextureTable::insertBefore(LLTextureInfo* a, LLTextureInfo* b)
{
	return (a->mName.compare(b->mName) < 0);
}


//void LLTextureTable::bubbleSort()
//{
//	mTextureInfoList.bubbleSortList();
//}

BOOL LLTextureTable::add(const std::string& name, const LLUUID &uuid, const std::string& description)
{
	LLTextureInfo *infop;

	infop = new LLTextureInfo(name, uuid, description);
	if (!infop) return FALSE;

	mTextureInfoList.addDataSorted(infop);

	return TRUE;
}


// *FIX: Since the table is sorted based on name, this could be vastly
// sped up by doing a binary search.
LLUUID LLTextureTable::getUUID(const std::string& name)
{
	LLTextureInfo *infop;

	for (infop = mTextureInfoList.getFirstData(); infop != NULL; infop = mTextureInfoList.getNextData() )
	{
		LLString name_lc = name;
		LLString::toLower(name_lc);
		if (name_lc == infop->mName)
		{
			return infop->mUUID;
		}
	}

	return LLUUID::null;
}


std::string LLTextureTable::getDesc(const std::string& name)
{
	LLTextureInfo *infop;

	for (infop = mTextureInfoList.getFirstData(); infop != NULL; infop = mTextureInfoList.getNextData() )
	{
		LLString name_lc = name;
		LLString::toLower(name_lc);
		if (name_lc == infop->mName)
		{
			return infop->mDescription;
		}
	}

	return LLString::null;
}


std::string LLTextureTable::getName(const LLUUID &id)
{
	LLTextureInfo *infop;

	for (infop = mTextureInfoList.getFirstData(); infop != NULL; infop = mTextureInfoList.getNextData() )
	{
		if (id == infop->mUUID)
		{
			return infop->mName;
		}
	}

	return LLString::null;
}


void LLTextureTable::deleteAll()
{
	// Clear out the list

	mTextureInfoList.deleteAllData();
}
