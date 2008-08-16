/** 
 * @file llpanelmoney.cpp
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

#include "llviewerprecompiledheaders.h"

#include "llpanelmoney.h"

#include "llfontgl.h"
#include "message.h"
#include "lltransactiontypes.h"

#include "llpanel.h"
#include "llbutton.h"
#include "llviewercontrol.h"
#include "llstatusbar.h"
#include "lluiconstants.h"
#include "llvieweruictrlfactory.h"
#include "viewer.h"

// XUI:translate
const char LOADING_STRING[]  = "Computing...";

S32 LLPanelAccount::sCurrentInterval = 0;

//---------------------------------------------------------------------------
// LLPanelAccount - the base class for all account panels
//---------------------------------------------------------------------------

// Construct with a dummy rectangle because tab containers will
// resize their panels.  Thus, merely need to make sure the follow
// flags are correctly set.
LLPanelAccount::LLPanelAccount(const LLUUID& request_id, S32 interval_days, S32 max_interval, const std::string& filename)
:	LLPanel("account_panel"),
	mRequestID( request_id ),
	mDataRequested( FALSE ),
	mIntervalDays( interval_days ),
	mMaxInterval( max_interval )
{
	sCurrentInterval = 0;

	gUICtrlFactory->buildPanel(this, filename);

	childSetText("editor", LOADING_STRING);

	childSetAction("earlier_btn", onClickEarlier, this);
	childSetAction("later_btn", onClickLater, this);
}


// public virtual
void LLPanelAccount::draw()
{
	if (!mDataRequested || (mDisplayedInterval != sCurrentInterval) )
	{
		sendRequest();
		refreshPanel();
		mDisplayedInterval = sCurrentInterval;
		mDataRequested = TRUE;
	}

	LLPanel::draw();
}

void LLPanelAccount::refreshPanel()
{
	// Used by some panels that have buttons and some that don't (so no warnings)
	if (hasChild("earlier_btn"))
	{
		childSetEnabled("earlier_btn",	sCurrentInterval < mMaxInterval);
	}
	if (hasChild("later_btn"))
	{
		childSetEnabled("later_btn",	sCurrentInterval > 0);
	}
}

// static
void LLPanelAccount::onClickEarlier(void *userdata)
{
	LLPanelAccount* self = (LLPanelAccount*)userdata;
	if (!self) return;

	if (self->sCurrentInterval < self->mMaxInterval)
	{
		self->childSetText("editor", LOADING_STRING);
		self->sCurrentInterval++;
		self->refreshPanel();
		self->setDataRequested(FALSE);
	}
}

// static
void LLPanelAccount::onClickLater(void *userdata)
{
	LLPanelAccount* self = (LLPanelAccount*)userdata;
	if (!self) return;

	if (self->sCurrentInterval > 0)
	{
		self->childSetText("editor", LOADING_STRING);
		self->sCurrentInterval--;
		self->refreshPanel();
		self->setDataRequested(FALSE);
	}
}


//---------------------------------------------------------------------------
// LLPanelAccountPlanning - the first, default view of money
//---------------------------------------------------------------------------
LLPanelAccountPlanning::LLPanelAccountPlanning(const LLUUID& request_id)
:	LLPanelAccount(request_id, SUMMARY_INTERVAL, SUMMARY_MAX, "panel_account_planning.xml")
{
	childSetText("editor", LOADING_STRING);
}

// virtual
void LLPanelAccountPlanning::sendRequest()
{
	LLMessageSystem* msg = gMessageSystem;

	msg->newMessageFast(_PREHASH_MoneySummaryRequest);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, agent_get_id() );
	msg->addUUIDFast(_PREHASH_SessionID, agent_get_session_id() );
	msg->nextBlockFast(_PREHASH_MoneyData);
	msg->addUUIDFast(_PREHASH_RequestID, mRequestID );
	msg->addS32Fast(_PREHASH_IntervalDays, mIntervalDays );
	msg->addS32Fast(_PREHASH_CurrentInterval, 0 );

	agent_send_reliable_message();

	childSetText("editor", LOADING_STRING);
}

// virtual
void LLPanelAccountPlanning::processReply(LLMessageSystem* msg, void** data)
{
	char line[MAX_STRING];
	LLString text;

	char start_date[MAX_STRING];
	char last_stipend_date[MAX_STRING];
	char next_stipend_date[MAX_STRING];
	S32 interval_days;
	S32 current_interval;
	S32 balance;
	S32 total_credits;
	S32 total_debits;
	S32 cur_object_tax;
	S32 cur_light_tax;
	S32 cur_land_tax;
	S32 cur_group_tax;
	S32 cur_parcel_dir_fee;
	S32 cur_total_tax;
	S32 proj_object_tax;
	S32 proj_light_tax;
	S32 proj_land_tax;
	S32 proj_group_tax;
	S32 proj_parcel_dir_fee;
	S32 proj_total_tax;
	S32 stipend;

	msg->getS32("MoneyData", "IntervalDays", interval_days );
	msg->getS32("MoneyData", "CurrentInterval", current_interval );
	msg->getS32("MoneyData", "Balance", balance );
	msg->getS32("MoneyData", "TotalCredits", total_credits );
	msg->getS32("MoneyData", "TotalDebits", total_debits );
	msg->getS32("MoneyData", "ObjectTaxCurrent", cur_object_tax );
	msg->getS32("MoneyData", "LightTaxCurrent", cur_light_tax );
	msg->getS32("MoneyData", "LandTaxCurrent", cur_land_tax );
	msg->getS32("MoneyData", "GroupTaxCurrent", cur_group_tax );
	msg->getS32("MoneyData", "ParcelDirFeeCurrent", cur_parcel_dir_fee );
	msg->getS32("MoneyData", "ObjectTaxEstimate", proj_object_tax );
	msg->getS32("MoneyData", "LightTaxEstimate", proj_light_tax );
	msg->getS32("MoneyData", "LandTaxEstimate", proj_land_tax );
	msg->getS32("MoneyData", "GroupTaxEstimate", proj_group_tax );
	msg->getS32("MoneyData", "ParcelDirFeeEstimate", proj_parcel_dir_fee );
	msg->getS32("MoneyData", "StipendEstimate", stipend );

	msg->getString("MoneyData", "StartDate", MAX_STRING, start_date);
	msg->getString("MoneyData", "LastTaxDate", MAX_STRING, last_stipend_date);
	msg->getString("MoneyData", "TaxDate", MAX_STRING, next_stipend_date);

	cur_total_tax = cur_object_tax + cur_light_tax + cur_land_tax + cur_group_tax +  cur_parcel_dir_fee;
	proj_total_tax = proj_object_tax + proj_light_tax + proj_land_tax + proj_group_tax + proj_parcel_dir_fee;

	if (interval_days != mIntervalDays || current_interval != sCurrentInterval)
	{
		llinfos << "Out of date summary packet " << interval_days << " " 
			<< current_interval << llendl;
		return;
	}

	// *NOTE: actually use the balance from the status bar, so the two
	// match.  This should not be necessary because the simulator no
	// longer buffers transaction. If status bar balance is zero,
	// assume this is because we haven't received the first balance
	// yet, and trust the database.
	if (0 != gStatusBar->getBalance())
	{
		balance = gStatusBar->getBalance();
	}

	sprintf(line, "Summary for this week, beginning on %s\n", start_date);
	text.append(line);

	if (current_interval == 0)
	{
		text.append(1, '\n');

		sprintf(line, "Your next stipend day is %s\n\n", next_stipend_date);
		text.append(line);
		sprintf(line, "%-24sL$%6d\n", "Balance", balance );
		text.append(line);

		text.append(1, '\n');
	}

	sprintf(line,     "%-24s  %6d\n", "Credits", total_credits);
	text.append(line);

	sprintf(line,     "%-24s  %6d\n", "Debits", total_debits);
	text.append(line);

	sprintf(line,     "%-24s  %6d\n", "Total Change", total_credits + total_debits);
	text.append(line);

	if (current_interval == 0)
	{
		text.append(1, '\n');

		text.append(1, '\n');

		sprintf(line, "%-24s  %6d\n", "Estimated Stipend", stipend );
		text.append(line);

		// Generate projected "wealth tax" based on current balance, projected stipend/bonus
	}
	else
	{
		text.append(1, '\n');
		text.append(1, '\n');

		sprintf(line, "%-24s  %6d\n", "Parcel Directory Fee", cur_parcel_dir_fee);
		text.append(line);
	}

	childSetText("editor", text);
}


//---------------------------------------------------------------------------
// LLPanelAccountDetails 
//---------------------------------------------------------------------------
LLPanelAccountDetails::LLPanelAccountDetails(const LLUUID& request_id)
:	LLPanelAccount(request_id, 
				   DETAILS_INTERVAL, 
				   DETAILS_MAX, 
				   "panel_account_details.xml")
{
	childSetText("editor", LOADING_STRING);
}

// virtual
void LLPanelAccountDetails::sendRequest()
{
	LLMessageSystem* msg = gMessageSystem;

	msg->newMessageFast(_PREHASH_MoneyDetailsRequest);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, agent_get_id() );
	msg->addUUIDFast(_PREHASH_SessionID, agent_get_session_id() );
	msg->nextBlockFast(_PREHASH_MoneyData);
	msg->addUUIDFast(_PREHASH_RequestID, mRequestID );
	msg->addS32Fast(_PREHASH_IntervalDays, mIntervalDays );
	msg->addS32Fast(_PREHASH_CurrentInterval, sCurrentInterval );

	agent_send_reliable_message();
}

// virtual
void LLPanelAccountDetails::processReply(LLMessageSystem* msg, void** data)
{
	char line[MAX_STRING];
	LLString text;

	char start_date[MAX_STRING];
	S32 interval_days;
	S32 current_interval;

	msg->getS32Fast(_PREHASH_MoneyData, _PREHASH_IntervalDays, interval_days );
	msg->getS32Fast(_PREHASH_MoneyData, _PREHASH_CurrentInterval, current_interval );
	msg->getStringFast(_PREHASH_MoneyData, _PREHASH_StartDate, MAX_STRING, start_date);

	if (interval_days != mIntervalDays || current_interval != sCurrentInterval)
	{
		llinfos << "Out of date details packet " << interval_days << " " 
			<< current_interval << llendl;
		return;
	}

	sprintf(line,     "%s\n\n", start_date);
	text.append(line);

	S32 total_amount = 0;
	S32 transactions = msg->getNumberOfBlocksFast(_PREHASH_HistoryData);
	for(S32 i = 0; i < transactions; i++)
	{
		S32			amount = 0;
		char		desc[MAX_STRING];

		msg->getStringFast(_PREHASH_HistoryData, _PREHASH_Description,	MAX_STRING, desc, i );
		msg->getS32Fast(_PREHASH_HistoryData, _PREHASH_Amount,		amount, i);

		if (amount != 0)
		{
			sprintf(line, "%-24s %6d\n", desc, amount );
			text.append(line);
		}
		else
		{
			// skip it
		}

		total_amount += amount;
	}

	text.append(1, '\n');

	sprintf(line, "%-24s %6d\n", "Total", total_amount );
	text.append(line);

	childSetText("editor", text);
}

//---------------------------------------------------------------------------
// LLPanelAccountTransactions
//---------------------------------------------------------------------------
LLPanelAccountTransactions::LLPanelAccountTransactions(const LLUUID& request_id)
:	LLPanelAccount(request_id, 
				   TRANSACTIONS_INTERVAL, 
				   TRANSACTIONS_MAX, 
				   "panel_account_transactions.xml")
{
	childSetText("editor", LOADING_STRING);
}

// virtual
void LLPanelAccountTransactions::sendRequest()
{
	LLMessageSystem* msg = gMessageSystem;

	msg->newMessageFast(_PREHASH_MoneyTransactionsRequest);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, agent_get_id() );
	msg->addUUIDFast(_PREHASH_SessionID, agent_get_session_id() );
	msg->nextBlockFast(_PREHASH_MoneyData);
	msg->addUUIDFast(_PREHASH_RequestID, mRequestID );
	msg->addS32Fast(_PREHASH_IntervalDays, mIntervalDays );
	msg->addS32Fast(_PREHASH_CurrentInterval, sCurrentInterval );

	agent_send_reliable_message();

	childSetText("editor", LOADING_STRING);
}

// virtual
void LLPanelAccountTransactions::processReply(LLMessageSystem* msg, void** data)
{
	char line[MAX_STRING];
	LLSD value = childGetValue("editor");
	std::string text = value.asString();

	char start_date[MAX_STRING];
	S32 interval_days;
	S32 current_interval;

	msg->getS32Fast(_PREHASH_MoneyData, _PREHASH_IntervalDays, interval_days );
	msg->getS32Fast(_PREHASH_MoneyData, _PREHASH_CurrentInterval, current_interval );
	msg->getStringFast(_PREHASH_MoneyData, _PREHASH_StartDate, MAX_STRING, start_date);

	if (interval_days != mIntervalDays || current_interval != sCurrentInterval)
	{
		llinfos << "Out of date details packet " << interval_days << " " 
			<< current_interval << llendl;
		return;
	}

	// If this is the first packet, clear the text, don't append.
	// Start with the date.
	if (text == LOADING_STRING)
	{
		text.clear();

		sprintf(line, "%s\n\n", start_date); 
		text.append(line);
	}

	S32 transactions = msg->getNumberOfBlocksFast(_PREHASH_HistoryData);
	if (transactions == 0)
	{
		text.append("(none)");
	}
	else
	{
		for(S32 i = 0; i < transactions; i++)
		{
			const S32 SHORT_STRING = 64;
			char		time[SHORT_STRING];
			S32			type = 0;
			S32			amount = 0;
			char		user[SHORT_STRING];
			char		item[SHORT_STRING];

			msg->getStringFast(_PREHASH_HistoryData, _PREHASH_Time,		SHORT_STRING, time, i);
			msg->getStringFast(_PREHASH_HistoryData, _PREHASH_User,		SHORT_STRING, user, i );
			msg->getS32Fast(_PREHASH_HistoryData, _PREHASH_Type,		type, i);
			msg->getStringFast(_PREHASH_HistoryData, _PREHASH_Item,		SHORT_STRING, item, i );
			msg->getS32Fast(_PREHASH_HistoryData, _PREHASH_Amount,	amount, i);

			if (amount != 0)
			{
				char* verb;

				switch(type)
				{
				case TRANS_OBJECT_SALE:
					verb = "bought";
					break;
				case TRANS_GIFT:
					verb = "paid you";
					break;
				case TRANS_PAY_OBJECT:
					verb = "paid into";
					break;
				case TRANS_LAND_PASS_SALE:
					verb = "bought pass to";
					break;
				case TRANS_EVENT_FEE:
					verb = "paid fee for event";
					break;
				case TRANS_EVENT_PRIZE:
					verb = "paid prize for event";
					break;
				default:
					verb = "";
					break;
				}

				sprintf(line, "%s %6d - %s %s %s\n", time, amount, user, verb, item);
				text.append(line);
			}
		}
	}

	childSetText("editor", text);
}
