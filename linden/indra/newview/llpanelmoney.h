/** 
 * @file llpanelmoney.h
 * @author James Cook
 * @brief Panels for Account History floater
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

#ifndef LL_LLPANELMONEY_H
#define LL_LLPANELMONEY_H

#include "llpanel.h"
#include "lluuid.h"

class LLMessageSystem;
class LLTextEditor;
class LLButton;

// Base class for all the panels.
class LLPanelAccount
: public LLPanel
{
public:
	LLPanelAccount(const LLUUID& request_id, S32 interval_days, S32 max_interval, const std::string& filename);

	void	setDataRequested(BOOL b)	{ mDataRequested = b; }

	virtual void draw();

	virtual void sendRequest() = 0;
	virtual void processReply(LLMessageSystem* msg, void** data) = 0;
	
	void refreshPanel();

	static void onClickEarlier(void* userdata);
	static void onClickLater(void* userdata);

protected:
	LLUUID			mRequestID;

	BOOL			mDataRequested;

	// Time between start and end day, inclusive.  7 = one week
	S32		mIntervalDays;

	// How many intervals back are we looking?
	static S32		sCurrentInterval;
	S32		mDisplayedInterval;

	S32		mMaxInterval;
};


class LLPanelAccountPlanning
:	public LLPanelAccount
{
public:
	LLPanelAccountPlanning(const LLUUID& request_id);

	virtual void sendRequest();
	virtual void processReply(LLMessageSystem* msg, void** data);
};


class LLPanelAccountDetails
:	public LLPanelAccount
{
public:
	LLPanelAccountDetails(const LLUUID& request_id);

	virtual void sendRequest();
	virtual void processReply(LLMessageSystem* msg, void** data);
};


class LLPanelAccountTransactions
:	public LLPanelAccount
{
public:
	LLPanelAccountTransactions(const LLUUID& request_id);

	virtual void sendRequest();
	virtual void processReply(LLMessageSystem* msg, void** data);
};

#endif
