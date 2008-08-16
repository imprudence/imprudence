/** 
 * @file llfloateraccounthistory.cpp
 * @brief LLFloaterAccountHistory class implementation
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

#include "llviewerprecompiledheaders.h"

#include "llfloateraccounthistory.h"

#include "llfontgl.h"
#include "message.h"
#include "llsecondlifeurls.h"
#include "lltransactiontypes.h"

#include "llagent.h"
#include "llalertdialog.h"
#include "llpanel.h"
#include "llbutton.h"
#include "llviewercontrol.h"
#include "llviewertexteditor.h"
#include "lluiconstants.h"
#include "llresizehandle.h"
#include "llstatusbar.h"
#include "lltabcontainervertical.h"
#include "llvieweruictrlfactory.h"
#include "llviewerwindow.h"
#include "llweb.h"

static const char MONEY_HX_TITLE[]  = "Account History";
static const S32 MONEY_HX_WIDTH  = 500;
static const S32 MONEY_HX_HEIGHT = 350;
static const S32 INSET = 4;


LLMap<LLUUID, LLFloaterAccountHistory*> LLFloaterAccountHistory::sInstances;




//---------------------------------------------------------------------------
// LLFloaterAccountHistory
// The container panel for all the panel views.
//---------------------------------------------------------------------------
// private
LLFloaterAccountHistory::LLFloaterAccountHistory()
:	LLFloater("money history floater")
{
	// Unique ID for transactions
	mFloaterID.generate();

	// Instance management
	LLFloaterAccountHistory::sInstances.addData(mFloaterID, this);

	// User interface construction
	gUICtrlFactory->buildFloater(this, "floater_account_history.xml");
	center();

	// Put resize handle on top
	removeChild(mResizeHandle[0]);
	addChild(mResizeHandle[0]);

	LLTabContainerCommon* tab = LLUICtrlFactory::getTabContainerByName(this, "tab");
	if (tab)
	{
		mPlanningPanel = new LLPanelAccountPlanning(mFloaterID);
		tab->addTabPanel( mPlanningPanel, "Planning" );

		mDetailsPanel = new LLPanelAccountDetails(mFloaterID);
		tab->addTabPanel( mDetailsPanel, "Details" );

		mTransactionsPanel = new LLPanelAccountTransactions(mFloaterID);
		tab->addTabPanel( mTransactionsPanel, "Sales/Gifts" );

		tab->selectFirstTab();
	}

	LLButton* btn = LLUICtrlFactory::getButtonByName(this, "details_btn");
	if (btn) btn->setClickedCallback(onClickDetails, this);
}


// private virtual
LLFloaterAccountHistory::~LLFloaterAccountHistory()
{
	LLFloaterAccountHistory::sInstances.removeData(mFloaterID);
}


// public static
void LLFloaterAccountHistory::show(void*)
{
	// Make sure the mouse is available for clicking
	if( gAgent.cameraMouselook() )
	{
		gAgent.changeCameraToDefault();
	}

	// Create the dialog and center on screen
	// This will show a default "computing..." message
	new LLFloaterAccountHistory();
}

// static 
void LLFloaterAccountHistory::processMoneySummaryReply(LLMessageSystem* msg, void** data)
{
	LLUUID agent_id;
	LLUUID floater_id;

	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );
	msg->getUUIDFast(_PREHASH_MoneyData, _PREHASH_RequestID, floater_id );

	if (agent_id != gAgent.getID())
	{
		llwarns << "Received money summary for wrong agent " << agent_id << llendl;
		return;
	}

	LLFloaterAccountHistory* floater = LLFloaterAccountHistory::sInstances.getIfThere(floater_id);
	if (!floater)
	{
		llwarns << "Received money summary for unknown window " << floater_id << llendl;
		return;
	}

	floater->open();
	floater->mPlanningPanel->processReply(msg, data);
}

// static 
void LLFloaterAccountHistory::processMoneyDetailsReply(LLMessageSystem* msg, void** data)
{
	LLUUID agent_id;
	LLUUID floater_id;

	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );
	msg->getUUIDFast(_PREHASH_MoneyData, _PREHASH_RequestID, floater_id );

	if (agent_id != gAgent.getID())
	{
		llwarns << "Received money details for wrong agent " << agent_id << llendl;
		return;
	}

	LLFloaterAccountHistory* floater = LLFloaterAccountHistory::sInstances.getIfThere(floater_id);
	if (!floater)
	{
		llwarns << "Received money details for unknown window " << floater_id << llendl;
		return;
	}

	floater->open();

	floater->mDetailsPanel->processReply(msg, data);
}

// static 
void LLFloaterAccountHistory::processMoneyTransactionsReply(LLMessageSystem* msg, void** data)
{
	LLUUID agent_id;
	LLUUID floater_id;

	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );
	msg->getUUIDFast(_PREHASH_MoneyData, _PREHASH_RequestID, floater_id );

	if (agent_id != gAgent.getID())
	{
		llwarns << "Received money transactions for wrong agent " << agent_id << llendl;
		return;
	}

	LLFloaterAccountHistory* floater = LLFloaterAccountHistory::sInstances.getIfThere(floater_id);
	if (!floater)
	{
		llwarns << "Received money transactions for unknown window " << floater_id << llendl;
		return;
	}

	floater->open();
	floater->mTransactionsPanel->processReply(msg, data);
}

void callback_load_url_account_transactions(S32 option, void*)
{
	if (option == 0)
	{
		LLWeb::loadURL(ACCOUNT_TRANSACTIONS_URL);
	}
}

// static
void LLFloaterAccountHistory::onClickDetails(void*)
{
	std::map<std::string,std::string> args;
	args["[URL]"] = ACCOUNT_TRANSACTIONS_URL;
	gViewerWindow->alertXml("LoadAccountTransactions", args, 
		callback_load_url_account_transactions, NULL);
}
