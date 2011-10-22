/** 
 * @file llaudiodecodemgr.h
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2009, Linden Research, Inc.
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

#ifndef LL_LLAUDIODECODEMGR_H
#define LL_LLAUDIODECODEMG_H

#include "stdtypes.h"

#include "lllinkedqueue.h"
#include "lluuid.h"

#include "llassettype.h"
#include "llframetimer.h"
#include <iterator>
class LLVFS;
class LLVorbisDecodeState;

class LLAudioDecodeMgr
{
public:
	LLAudioDecodeMgr();
	~LLAudioDecodeMgr();

	void processQueue(const F32 num_secs = 0.005);
	BOOL addDecodeRequest(const LLUUID &uuid);
	void addAudioRequest(const LLUUID &uuid);
	
protected:
	class Impl;
	Impl* mImpl;
};

extern LLAudioDecodeMgr *gAudioDecodeMgrp;

#endif
