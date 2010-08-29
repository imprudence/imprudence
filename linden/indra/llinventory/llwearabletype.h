/** 
 * @file llwearable.h
 * @brief EWearableType enum definition
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#ifndef LL_LLWEARABLETYPE_H
#define LL_LLWEARABLETYPE_H

enum	EWearableType  // If you change this, update LLWearable::getTypeName(), getTypeLabel(), and LLVOAvatar::getTEWearableType()
{
	WT_SHAPE	= 0,
	WT_SKIN		= 1,
	WT_HAIR		= 2,
	WT_EYES		= 3,
	WT_SHIRT	= 4,
	WT_PANTS	= 5,
	WT_SHOES	= 6,
	WT_SOCKS	= 7,
	WT_JACKET	= 8,
	WT_GLOVES	= 9,
	WT_UNDERSHIRT = 10,
	WT_UNDERPANTS = 11,
	WT_SKIRT	= 12,
	WT_ALPHA	= 13,
	WT_TATTOO	= 14,
	WT_COUNT	= 15,
	WT_INVALID	= 255
};

#endif  // LL_LLWEARABLETYPE_H
