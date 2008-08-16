/** 
 * @file llfloateraccounthistory.h
 * @brief LLFloaterAccountHistory class definition
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

/**
 * The user's account history, including all Linden$ transactions from
 * the transaction table.
 */

#ifndef LL_LLFLOATERACCOUNTHISTORY_H
#define LL_LLFLOATERACCOUNTHISTORY_H

#include "llfloater.h"
#include "llmap.h"
#include "lluuid.h"
#include "llpanelmoney.h"

class LLMessageSystem;
class LLTabContainer;
class LLTabContainerVertical;
class LLTextEditor;
class LLButton;

class LLFloaterAccountHistory
:	public LLFloater
{
public:
	static void show(void*);

	// Message system callbacks
	static void processMoneySummaryReply(LLMessageSystem* msg, void**);
	static void processMoneyDetailsReply(LLMessageSystem* msg, void**);
	static void processMoneyTransactionsReply(LLMessageSystem* msg, void**);

private:
	// Must construct by calling show().
	LLFloaterAccountHistory();
	virtual ~LLFloaterAccountHistory();

	static void onClickDetails(void*);

private:
	// Each money history window has its own id, which it uses
	// for transactions and to allow more than one window to
	// be open.
	LLUUID				mFloaterID;

	// The actual information is encoded in panels in this
	// tab container.
	LLTabContainerVertical*		mTab;

	LLPanelAccountPlanning*		mPlanningPanel;
	LLPanelAccountDetails*		mDetailsPanel;
	LLPanelAccountTransactions*	mTransactionsPanel;
	LLButton* mBtnDetails;

	static LLMap<LLUUID, LLFloaterAccountHistory*> sInstances;
};

#endif
