/** 
 * @file llpanelgroupvoting.h
 * @brief LLPanelGroupVoting class definition.
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

#include "llviewerprecompiledheaders.h"

#include "roles_constants.h"

#include "lllineeditor.h"
#include "lltextbox.h"
#include "lltexteditor.h"
#include "llscrolllistctrl.h"
#include "llradiogroup.h"
#include "llspinctrl.h"
#include "llpanelgroupvoting.h"
#include "llnamelistctrl.h"
#include "llbutton.h"

#include "llagent.h"
#include "llfocusmgr.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"

class LLPanelGroupVoting::impl
{
public:
	impl(const LLUUID& group_id);
	~impl();

	static void onClickCreateProposal(void* userdata);
	static void onClickYes(void *userdata);
	static void onClickNo(void *userdata);
	static void onClickAbstain(void *userdata);
	static void onClickSubmitProposal(void *userdata);
	static void onClickCancelProposal(void *userdata);
	static void onClickViewProposalList(void *userdata);
	static void onClickViewProposalItem(void *userdata);
	static void onDoubleClickProposal(void *userdata);

	static void onDoubleClickHistoryItem(void *userdata);
	static void onClickViewHistoryItem(void *userdata);
	static void onClickViewHistoryList(void *userdata);

	void sendGroupProposalsRequest(const LLUUID& group_id);
	void sendStartGroupProposal();
	void sendGroupProposalBallot(const char* vote);
	void sendGroupVoteHistoryRequest(const LLUUID& group_id);

	void setEnableCreateProposal();
	void setEnableVoteProposal();
	void setEnableListProposals();
	void setEnableHistoryItem();
	void setEnableHistoryList();

	void updateQuorumText();
	void addPendingActiveScrollListItem(unsigned int current,
										unsigned int expected,
										EAddPosition pos);
	void addPendingHistoryScrollListItem(unsigned int current,
										 unsigned int expected,
										 EAddPosition pos);
	void addNoActiveScrollListItem(EAddPosition pos);
	void addNoHistoryScrollListItem(EAddPosition pos);
													 

	static void processGroupActiveProposalItemReply(LLMessageSystem* msg,
													void** data);
	static void processGroupVoteHistoryItemReply(LLMessageSystem* msg,
												 void** data);

public:
	LLUUID mGroupID;
	LLUUID mProposalID;

	LLTextBox		*mQuorumText;
	LLSpinCtrl		*mQuorum;
	LLTextBox		*mQuorumLbl;
	LLSpinCtrl		*mDuration;
	LLTextBox		*mDurationLbl;
	LLTextBox       *mDurationText;
	LLRadioGroup	*mMajority;
	LLTextBox		*mMajorityLbl;
	LLTextBox		*mInstructions;
	LLTextBox       *mProposalHeader;
	LLTextBox       *mProposalLbl;
	LLTextBox		*mStartLbl;
	LLTextBox	    *mEndLbl;
	LLTextBox	    *mStartDate;
	LLTextBox	    *mEndDate;

	LLButton *mBtnYes;
	LLButton *mBtnNo;
	LLButton *mBtnAbstain;

	LLScrollListCtrl *mProposals;
	LLTextEditor     *mProposalText;
	LLButton	     *mBtnCreateProposal;
	LLButton	     *mBtnSubmitProposal;
	LLButton         *mBtnCancelProposal;
	LLButton	     *mBtnViewProposalList;
	LLButton	     *mBtnViewProposalItem;

	LLScrollListCtrl *mVotesHistory;
	LLTextBox	     *mVotesHistoryLbl;
	LLTextEditor	 *mVoteHistoryText;
	LLTextBox        *mVoteHistoryTextLbl;
	LLButton		 *mBtnViewHistoryList;
	LLButton		 *mBtnViewHistoryItem;

	std::string mViewProposalHeaderText;
	std::string mCreateProposalHeaderText;
	std::string mVoteProposalHeaderText;
	std::string mEmptyProposalText;

	int	mNumGroupMembers;

	std::vector<LLSD> mActiveReceived;
	std::vector<LLSD> mHistoryReceived;

	int mProposalColumnWidths[10];
	int mHistoryColumnWidths[10];

	LLUUID mProposalTransID;
	LLUUID mHistoryTransID;

	static std::map<LLUUID, LLPanelGroupVoting::impl*> sGroupIDs;
};

//****************************************
//** LLPanelGroupVoting::impl Functions **
//****************************************

LLPanelGroupVoting::impl::impl(const LLUUID& group_id)
{
	mGroupID = group_id;

	mNumGroupMembers = 0;

	mProposalTransID = LLUUID::null;
	mHistoryTransID  = LLUUID::null;

	mProposalColumnWidths[0] = -1;
	mProposalColumnWidths[1] = 210;
	mProposalColumnWidths[2] = 120;
	mProposalColumnWidths[3] = -1;
	mProposalColumnWidths[4] = 30;
	for (int index = 5; index < 10; index++)
	{
		mProposalColumnWidths[index] = -1;
	}

	mHistoryColumnWidths[0] = -1;
	mHistoryColumnWidths[1] = 210; // vote text sans non-printable
	mHistoryColumnWidths[2] = 120;
	mHistoryColumnWidths[3] = -1;
	mHistoryColumnWidths[4] = -1;
	mHistoryColumnWidths[5] = -1;	// full vote text

	//problem what if someone has both the group floater open and the finder
	//open to the same group?  Some maps that map group ids to panels
	//will then only be working for the last panel for a given group id :(
	sGroupIDs[group_id] = this;

	mQuorumText     = NULL;
	mQuorum         = NULL;
	mQuorumLbl      = NULL;
	mDuration       = NULL;
	mDurationLbl    = NULL;
	mDurationText   = NULL;
	mMajority       = NULL;
	mMajorityLbl    = NULL;
	mInstructions   = NULL;
	mProposalHeader = NULL;
	mProposalLbl    = NULL;
	mStartLbl       = NULL;
	mEndLbl         = NULL;
	mStartDate      = NULL;
	mEndDate        = NULL;

	mBtnYes     = NULL;
	mBtnNo      = NULL;
	mBtnAbstain = NULL;

	mProposals           = NULL;
	mProposalText        = NULL;
	mBtnCreateProposal   = NULL;
	mBtnSubmitProposal   = NULL;
	mBtnViewProposalList = NULL;
	mBtnViewProposalItem = NULL;

	mVotesHistory       = NULL;
	mVotesHistoryLbl    = NULL;
	mVoteHistoryText    = NULL;
	mVoteHistoryTextLbl = NULL;
	mBtnViewHistoryList = NULL;
	mBtnViewHistoryItem = NULL;
}

LLPanelGroupVoting::impl::~impl()
{
	sGroupIDs.erase(mGroupID);
}

void LLPanelGroupVoting::impl::setEnableListProposals()
{
	//we make the assumption here that anyone who has the start
	//or vote powers should also have the view powers
	
	/* This power was removed to make group roles simpler
	if (!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VIEW) &&
		!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VOTE) &&
		!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_START) )
	*/
	if (!gAgent.isInGroup(mGroupID))
		return;


	mProposalHeader->setText(mViewProposalHeaderText);
	mInstructions->setText("Double Click on the proposal you would like to vote on or press\n"
						   "Create Proposal to create a new proposal.");
	mProposalLbl->setText("Proposal                                                         Voting Ends                    Voted?");
	mProposals->setVisible(TRUE);
	mProposals->setBgSelectedColor(gColors.getColor("ScrollSelectedBGColor"));
	mProposalText->setVisible(FALSE);
	mBtnYes->setEnabled(FALSE);
	mBtnYes->setVisible(FALSE);
	mBtnNo->setEnabled(FALSE);
	mBtnNo->setVisible(FALSE);
	mBtnAbstain->setEnabled(FALSE);
	mBtnAbstain->setVisible(FALSE);
	mBtnSubmitProposal->setEnabled(FALSE);
	mBtnSubmitProposal->setVisible(FALSE);
	mBtnCancelProposal->setEnabled(FALSE);
	mBtnCancelProposal->setVisible(FALSE);

	mBtnViewProposalItem->setEnabled(TRUE);
	mBtnViewProposalItem->setVisible(TRUE);

	mBtnViewProposalList->setEnabled(FALSE);
	mBtnViewProposalList->setVisible(FALSE);

	mQuorumLbl->setVisible(FALSE);
	mQuorum->setEnabled(FALSE);
	mQuorum->setVisible(FALSE);
	mQuorumText->setVisible(FALSE);

	mDurationLbl->setVisible(FALSE);
	mDuration->setEnabled(FALSE);
	mDuration->setVisible(FALSE);
	mDurationText->setVisible(FALSE);

	mMajorityLbl->setVisible(FALSE);
	mMajority->setEnabled(FALSE);
	mMajority->setVisible(FALSE);
	mStartLbl->setVisible(FALSE);
	mStartDate->setVisible(FALSE);
	mEndLbl->setVisible(FALSE);
	mEndDate->setVisible(FALSE);

	//check to see if the agent can create proposals when enabling the
	//create proposal button
	BOOL can_create = gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_START);

	mBtnCreateProposal->setEnabled(can_create);
	mBtnCreateProposal->setVisible(can_create);
}

void LLPanelGroupVoting::impl::setEnableVoteProposal()
{
	if (!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VOTE))
		return;

	LLScrollListItem *item = mProposals->getFirstSelected();
	if (item)
	{
		std::string already_voted;
		std::string vote_cast;

		mProposalID = item->getUUID();
		// col 0: index id
		mProposalText->setText(item->getColumn(1)->getText()); //proposal text
		mEndDate->setText(item->getColumn(2)->getText()); //end date
		// col 3: Vote Type
		already_voted = item->getColumn(4)->getText(); //already voted
		mStartDate->setText(item->getColumn(5)->getText()); //start date
		vote_cast = item->getColumn(6)->getText(); // Vote Cast
		// col 7: Vote Initiator
		mQuorum->set((F32)atoi(item->getColumn(8)->getText().c_str())); //quorum
		F32 majority = (F32)atof(item->getColumn(9)->getText().c_str()); //majority

		if(majority == 0.0f)
		{	// Select the Simple Majority
			mMajority->setSelectedIndex(0);
		}
		else if (majority == 1.0f)
		{
			//Select Unanimous
			mMajority->setSelectedIndex(2);
		}
		else
		{
			//Select 2/3 Majority
			mMajority->setSelectedIndex(1);
		}

		if (already_voted == "Yes")
		{
			char message[MAX_STRING];		/*Flawfinder: ignore*/
			snprintf(message, MAX_STRING, "You have voted: %s ", vote_cast.c_str());			/* Flawfinder: ignore */
			mInstructions->setText(message);

			mBtnYes->setEnabled(FALSE);
			mBtnNo->setEnabled(FALSE);
			mBtnAbstain->setEnabled(FALSE);
		}
		else
		{
			mInstructions->setText("Press the Yes or No button to vote on the proposal, \n"
									"or choose to Abstain voting on this proposal.");

			mBtnYes->setEnabled(TRUE);
			mBtnNo->setEnabled(TRUE);
			mBtnAbstain->setEnabled(TRUE);
		}

		mProposalHeader->setText(mVoteProposalHeaderText);
		mProposalLbl->setText("Proposal Description");
		mProposals->setVisible(FALSE);
		mProposalText->setEnabled(FALSE);
		mProposalText->setVisible(TRUE);
		mBtnYes->setVisible(TRUE);
		mBtnNo->setVisible(TRUE);
		mBtnAbstain->setVisible(TRUE);
		mBtnSubmitProposal->setEnabled(FALSE);
		mBtnSubmitProposal->setVisible(FALSE);
		mBtnCreateProposal->setEnabled(FALSE);
		mBtnCreateProposal->setVisible(FALSE);
		mBtnCancelProposal->setEnabled(FALSE);
		mBtnCancelProposal->setVisible(FALSE);

		mBtnViewProposalItem->setEnabled(FALSE);
		mBtnViewProposalItem->setVisible(FALSE);

		mBtnViewProposalList->setEnabled(TRUE);
		mBtnViewProposalList->setVisible(TRUE);

		mQuorumLbl->setVisible(TRUE);
		mQuorum->setEnabled(FALSE);
		mQuorum->setVisible(TRUE);
		mQuorumText->setVisible(TRUE);

		mDurationLbl->setVisible(FALSE);
		mDuration->setEnabled(FALSE);
		mDuration->setVisible(FALSE);
		mDurationText->setVisible(FALSE);

		mMajorityLbl->setVisible(TRUE);
		mMajority->setEnabled(FALSE);
		mMajority->setVisible(TRUE);
		mStartLbl->setVisible(TRUE);
		mStartDate->setVisible(TRUE);
		mEndLbl->setVisible(TRUE);
		mEndDate->setVisible(TRUE);
	}
}

void LLPanelGroupVoting::impl::setEnableCreateProposal()
{
	if ( !gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_START) )
		 return;

	mProposalHeader->setText(mCreateProposalHeaderText);
	mInstructions->setText("Type the Proposal Description for the new proposal. You can change the\nQuorum, Duration and Majority required to pass the proposal.");
	mProposalLbl->setText("Proposal Description");
	mProposals->setVisible(FALSE); 
	mProposalText->setEnabled(TRUE);
	mProposalText->setVisible(TRUE);
	mProposalText->setText("");
	mBtnYes->setEnabled(FALSE);
	mBtnYes->setVisible(FALSE);
	mBtnNo->setEnabled(FALSE);
	mBtnNo->setVisible(FALSE);
	mBtnAbstain->setEnabled(FALSE);
	mBtnAbstain->setVisible(FALSE);
	mBtnSubmitProposal->setEnabled(TRUE);
	mBtnSubmitProposal->setVisible(TRUE);
	mBtnCancelProposal->setEnabled(TRUE);
	mBtnCancelProposal->setVisible(TRUE);
	mBtnCreateProposal->setEnabled(FALSE);
	mBtnCreateProposal->setVisible(FALSE);

	mBtnViewProposalItem->setEnabled(FALSE);
	mBtnViewProposalItem->setVisible(FALSE);

	mBtnViewProposalList->setEnabled(FALSE);
	mBtnViewProposalList->setVisible(FALSE);

	mQuorumLbl->setVisible(TRUE);
	mQuorum->setEnabled(TRUE);
	mQuorum->setVisible(TRUE);
	mQuorumText->setVisible(TRUE);

	mDurationLbl->setVisible(TRUE);
	mDuration->setEnabled(TRUE);
	mDuration->setVisible(TRUE);
	mDurationText->setVisible(TRUE);

	mMajorityLbl->setVisible(TRUE);
	mMajority->setEnabled(TRUE);
	mMajority->setVisible(TRUE);
	mStartLbl->setVisible(FALSE);
	mStartDate->setVisible(FALSE);
	mEndLbl->setVisible(FALSE);
	mEndDate->setVisible(FALSE);

	if (mQuorum->get() == 0)
	{
		mQuorum->set((F32)mNumGroupMembers);
	}
	mQuorum->setMaxValue((F32)mNumGroupMembers);
}

void LLPanelGroupVoting::impl::setEnableHistoryList()
{
	//we make the assumption here that anyone who has the start
	//or vote powers should also have the view powers

	/* This power was removed to make group roles simpler
	if (!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VIEW) &&
		!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VOTE) &&
		!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_START) )
		return;
	*/
	if (!gAgent.isInGroup(mGroupID))
		return;

	mVotesHistoryLbl->setVisible(TRUE);
	mVotesHistory->setVisible(TRUE);
	mVotesHistory->setEnabled(TRUE);

	mVoteHistoryTextLbl->setVisible(FALSE);
	mVoteHistoryText->setVisible(FALSE);	
	mBtnViewHistoryList->setVisible(FALSE);

	mBtnViewHistoryItem->setVisible(TRUE);
	mBtnViewHistoryItem->setEnabled(TRUE);
}

void LLPanelGroupVoting::impl::setEnableHistoryItem()
{
	//we make the assumption here that anyone who has the start
	//or vote powers should also have the view powers
	
	/* This power was removed to make group roles simpler
	if (!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VIEW) &&
		!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VOTE) &&
		!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_START) )
		return;
	*/
	
	if (!gAgent.isInGroup(mGroupID))
		return;

	LLScrollListItem *item = mVotesHistory->getFirstSelected();
	// Get full text, not stripped version.
	const LLScrollListCell *cell = item->getColumn(5);
	mVoteHistoryText->setText(cell->getText());

	mVotesHistoryLbl->setVisible(FALSE);
	mVotesHistory->setVisible(FALSE);

	mVoteHistoryTextLbl->setVisible(TRUE);
	mVoteHistoryText->setVisible(TRUE);	
	mVoteHistoryText->setEnabled(FALSE);

	mBtnViewHistoryList->setVisible(TRUE);
	mBtnViewHistoryList->setEnabled(TRUE);
	mBtnViewHistoryItem->setVisible(FALSE);
}

void LLPanelGroupVoting::impl::sendGroupProposalsRequest(const LLUUID& group_id)
{
	//we make the assumption here that anyone who has the start
	//or vote powers should also have the view powers
	
	/* This power was removed to make group roles simpler
	if (!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VIEW) &&
		!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VOTE) &&
		!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_START) )
		return;
	*/
	if ( !gAgent.isInGroup(mGroupID) )
		return;

	mProposalTransID.generate();
	mProposals->deleteAllItems(); //this should delete all the objects
	mActiveReceived.clear();

	//fill in some text so the user will at least know that
	//we're pining the server in high latency situations
	addPendingActiveScrollListItem(0, 0, ADD_BOTTOM);
	mProposals->setCanSelect(FALSE);

	LLMessageSystem *msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_GroupActiveProposalsRequest);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID() );
	msg->nextBlockFast(_PREHASH_GroupData);
	msg->addUUIDFast(_PREHASH_GroupID, group_id );
	msg->nextBlockFast(_PREHASH_TransactionData);
	msg->addUUIDFast(_PREHASH_TransactionID, mProposalTransID);
	gAgent.sendReliableMessage();
}

void LLPanelGroupVoting::impl::sendStartGroupProposal()
{
	if ( !gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_START) )
		return;

	F32 majority = 0.f;
	S32 duration_seconds = (S32)mDuration->get()*24*60*60; // duration days * 24hours/day * 60min/hr * 60sec/min

	switch (mMajority->getSelectedIndex())
	{
	case 0: // Simple Majority
		majority = 0.f;
		break;
	case 1: // 2/3 Majority
		majority = 2.f/3.f;
		break;
	case 2: // Unanimous
		majority = 1.0f;
		break;
	}

	S32 quorum = llfloor(mQuorum->get());

	LLMessageSystem *msg = gMessageSystem;

	msg->newMessageFast(_PREHASH_StartGroupProposal);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID() );

	msg->nextBlockFast(_PREHASH_ProposalData);
	msg->addUUIDFast(_PREHASH_GroupID, mGroupID);
	msg->addF32Fast(_PREHASH_Majority, majority );
	msg->addS32Fast(_PREHASH_Quorum, quorum );
	msg->addS32Fast(_PREHASH_Duration, duration_seconds );
	msg->addStringFast(_PREHASH_ProposalText, mProposalText->getText().c_str());

	gAgent.sendReliableMessage();
}

void LLPanelGroupVoting::impl::sendGroupProposalBallot(const char* vote)
{
	if ( !gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VOTE) )
		return;

	LLMessageSystem *msg = gMessageSystem;

	msg->newMessageFast(_PREHASH_GroupProposalBallot);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID() );

	msg->nextBlockFast(_PREHASH_ProposalData);
	msg->addUUIDFast(_PREHASH_ProposalID, mProposalID);
	msg->addUUIDFast(_PREHASH_GroupID, mGroupID);
	msg->addStringFast(_PREHASH_VoteCast, vote);

	gAgent.sendReliableMessage();
}

void LLPanelGroupVoting::impl::sendGroupVoteHistoryRequest(const LLUUID& group_id)
{
	//we make the assumption here that anyone who has the start
	//or vote powers should also have the view powers
	
	/* This power was removed to make group roles simpler
	if (mAwaitingFirstHistoryItem ||
		(!gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VIEW) &&
		 !gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_VOTE) &&
		 !gAgent.hasPowerInGroup(mGroupID, GP_PROPOSAL_START)) )
		return;
	*/

	if (!gAgent.isInGroup(mGroupID))
		return;

	mVotesHistory->deleteAllItems();
	mHistoryTransID.generate();
	mHistoryReceived.clear();

	//add some text so the user knows we're doing something
	addPendingHistoryScrollListItem(0, 0, ADD_BOTTOM);
	mVotesHistory->setCanSelect(FALSE);

	LLMessageSystem *msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_GroupVoteHistoryRequest);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID() );
	msg->nextBlockFast(_PREHASH_GroupData);
	msg->addUUIDFast(_PREHASH_GroupID, group_id );
	msg->nextBlockFast(_PREHASH_TransactionData);
	msg->addUUIDFast(_PREHASH_TransactionID, mHistoryTransID);
	gAgent.sendReliableMessage();
}

void LLPanelGroupVoting::impl::updateQuorumText()
{
	if ( mQuorumText )
	{
		//update the quorum count
		char quorum_text[MAX_STRING];		/*Flawfinder: ignore*/
		snprintf(quorum_text, MAX_STRING,					/* Flawfinder: ignore */
				" out of %d members must vote",
				mNumGroupMembers);
		mQuorumText->setText(quorum_text);
	}
}

void LLPanelGroupVoting::impl::addPendingActiveScrollListItem(unsigned int current,
															  unsigned int expected,
															  EAddPosition pos)
{
	std::stringstream pending;
	pending << "Retrieving active proposals ("
			<< current
			<< "\\" << expected  << ")";

	LLSD row;
	row["columns"][0]["value"] = pending.str();
	row["columns"][0]["font"] = "SANSSERIF_SMALL";
	mProposals->addElement(row, pos);
}

void LLPanelGroupVoting::impl::addNoActiveScrollListItem(EAddPosition pos)
{
	LLSD row;
	row["columns"][0]["value"] = "There are currently no active proposals";
	row["columns"][0]["font"] = "SANSSERIF_SMALL";
	mProposals->addElement(row, pos);
}

void LLPanelGroupVoting::impl::addNoHistoryScrollListItem(EAddPosition pos)
{
	LLSD row;
	row["columns"][0]["value"] = "There are currently no archived proposals";
	row["columns"][0]["font"] = "SANSSERIF_SMALL";
	mVotesHistory->addElement(row, pos);
}

void LLPanelGroupVoting::impl::addPendingHistoryScrollListItem(unsigned int current,
															  unsigned int expected,
															  EAddPosition pos)
{
	std::stringstream pending;
	pending << "Retrieving archived proposals ("
			<< current
			<< "\\" << expected  << ")";

	LLSD row;
	row["columns"][0]["value"] = pending.str();
	row["columns"][0]["font"] = "SANSSERIF_SMALL";

	mVotesHistory->addElement(row, pos);
}
																		

//static
std::map<LLUUID, LLPanelGroupVoting::impl*> LLPanelGroupVoting::impl::sGroupIDs;

//static
void LLPanelGroupVoting::impl::processGroupActiveProposalItemReply(LLMessageSystem *msg, void**)
{
	LLUUID agent_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );
	if (gAgent.getID() != agent_id)
	{
		llwarns << "Got active group proposals reply for another agent!"
			<< " Probably a userserver bug!" << llendl;
		return;
	}

	LLUUID group_id;
	LLUUID trans_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_GroupID, group_id );
	impl* self = get_if_there(sGroupIDs, group_id, (impl*) NULL);
	if (!self) 
	{
		return;
	}

	msg->getUUIDFast(_PREHASH_TransactionData,
					 _PREHASH_TransactionID,
					 trans_id);

	if ( self->mProposalTransID != trans_id ) return;


	U32 num_expected;
	const S32 MAX_STRING_NUM_LEN = 20;
	char item_num_string[DB_TERSE_DATETIME_BUF_SIZE];		/*Flawfinder: ignore*/
	char proposal_text[DB_VOTE_TEXT_BUF_SIZE];		/*Flawfinder: ignore*/
	char vote_cast[DB_VOTE_RESULT_BUF_SIZE];		/*Flawfinder: ignore*/
	char start_datetime[DB_DATETIME_BUF_SIZE];		/*Flawfinder: ignore*/
	char end_datetime[DB_DATETIME_BUF_SIZE];		/*Flawfinder: ignore*/
	char vote_type[DB_VOTE_TYPE_BUF_SIZE];		/*Flawfinder: ignore*/
	char majority_text[MAX_STRING_NUM_LEN];		/*Flawfinder: ignore*/
	char quorum_text[MAX_STRING_NUM_LEN];		/*Flawfinder: ignore*/
	char vote_initiator_string[UUID_STR_LENGTH];		/*Flawfinder: ignore*/

	LLUUID vote_id;
	LLUUID vote_initiator;

	F32  majority;
	S32  quorum;
	BOOL already_voted;
	S32 num_proposals = msg->getNumberOfBlocksFast(_PREHASH_ProposalData);
	
	msg->getU32Fast(_PREHASH_TransactionData,
					_PREHASH_TotalNumItems,
					num_expected);

	//the first item should be indicative that we
	//are currently receiving information or have it all
	self->mProposals->deleteAllItems();
	// make sure column indices are correct when populating with real data
	self->mProposals->clearColumns();

	for (int i = 0; i < num_proposals; i++)
	{
		msg->getStringFast(_PREHASH_ProposalData, _PREHASH_TerseDateID, DB_TERSE_DATETIME_BUF_SIZE, item_num_string, i );
		msg->getUUIDFast(_PREHASH_ProposalData, _PREHASH_VoteID,	vote_id, i );
		msg->getStringFast(_PREHASH_ProposalData, _PREHASH_StartDateTime, DB_DATETIME_BUF_SIZE, start_datetime, i );
		msg->getStringFast(_PREHASH_ProposalData, _PREHASH_EndDateTime, DB_DATETIME_BUF_SIZE, end_datetime, i );
		msg->getUUIDFast(_PREHASH_ProposalData, _PREHASH_VoteInitiator, vote_initiator, i );
		msg->getBOOLFast(_PREHASH_ProposalData,_PREHASH_AlreadyVoted, already_voted, i );
		msg->getStringFast(_PREHASH_ProposalData,_PREHASH_VoteCast, DB_VOTE_RESULT_BUF_SIZE, vote_cast, i );
		msg->getStringFast(_PREHASH_ProposalData, _PREHASH_ProposalText, DB_VOTE_TEXT_BUF_SIZE, proposal_text, i );
		msg->getF32Fast(_PREHASH_ProposalData, _PREHASH_Majority, majority, i );
		msg->getS32Fast(_PREHASH_ProposalData, _PREHASH_Quorum, quorum, i );

		vote_initiator.toString(vote_initiator_string);
		snprintf(majority_text, MAX_STRING_NUM_LEN, "%f", majority);			/* Flawfinder: ignore */
		snprintf(quorum_text, MAX_STRING_NUM_LEN, "%i", quorum);			/* Flawfinder: ignore */

		LLSD row;
		S32 index = 0;
		row["id"] = vote_id;
		row["columns"][0]["column"] = "item_num";
		row["columns"][0]["value"] = item_num_string;
		row["columns"][0]["font"] = "SANSSERIF_SMALL";
		row["columns"][0]["width"] = self->mProposalColumnWidths[index++];

		row["columns"][1]["column"] = "proposal_text";
		row["columns"][1]["value"] = proposal_text;
		row["columns"][1]["font"] = "SANSSERIF_SMALL";
		row["columns"][1]["width"] = self->mProposalColumnWidths[index++];

		row["columns"][2]["column"] = "end_datetime";
		row["columns"][2]["value"] = end_datetime;
		row["columns"][2]["font"] = "SANSSERIF_SMALL";
		row["columns"][2]["width"] = self->mProposalColumnWidths[index++];

		row["columns"][3]["column"] = "vote_type";
		row["columns"][3]["value"] = vote_type;
		row["columns"][3]["font"] = "SANSSERIF_SMALL";
		row["columns"][3]["width"] = self->mProposalColumnWidths[index++];

		row["columns"][4]["column"] = "already_voted";
		row["columns"][4]["value"] = already_voted ? "Yes" : "No";
		row["columns"][4]["font"] = "SANSSERIF_SMALL";
		row["columns"][4]["width"] = self->mProposalColumnWidths[index++];

		row["columns"][5]["column"] = "start_datetime";
		row["columns"][5]["value"] = start_datetime;
		row["columns"][5]["font"] = "SANSSERIF_SMALL";
		row["columns"][5]["width"] = self->mProposalColumnWidths[index++];

		row["columns"][6]["column"] = "vote_cast";
		row["columns"][6]["value"] = vote_cast;
		row["columns"][6]["font"] = "SANSSERIF_SMALL";
		row["columns"][6]["width"] = self->mProposalColumnWidths[index++];

		row["columns"][7]["column"] = "vote_initator_string";
		row["columns"][7]["value"] = vote_initiator_string;
		row["columns"][7]["font"] = "SANSSERIF_SMALL";
		row["columns"][7]["width"] = self->mProposalColumnWidths[index++];

		row["columns"][8]["column"] = "quorum_text";
		row["columns"][8]["value"] = quorum_text;
		row["columns"][8]["font"] = "SANSSERIF_SMALL";
		row["columns"][8]["width"] = self->mProposalColumnWidths[index++];

		row["columns"][9]["column"] = "majority_text";
		row["columns"][9]["value"] = majority_text;
		row["columns"][9]["font"] = "SANSSERIF_SMALL";
		row["columns"][9]["width"] = self->mProposalColumnWidths[index++];
		
		self->mActiveReceived.push_back(row);
	}

	int received = self->mActiveReceived.size();
	if ( num_expected == 0 )
	{
		//we are expecting 0 items, put up a message indicating tehre are
		//no active proposals and make the scroll list unselectable
		self->addNoActiveScrollListItem(ADD_BOTTOM);
		self->mProposals->setCanSelect(FALSE);
	}
	else if ( (U32)received != num_expected )
	{
		self->addPendingActiveScrollListItem(received,
											 num_expected,
											 ADD_BOTTOM);
		self->mProposals->setCanSelect(FALSE);
	}
	else
	{
		//all done display all of the items
		std::vector<LLSD>::iterator it = self->mActiveReceived.begin();
		std::vector<LLSD>::iterator end = self->mActiveReceived.end();

		for (; it != end; it++)
		{
			self->mProposals->addElement((*it), ADD_SORTED);
		}

		self->mProposals->setCanSelect(TRUE);
	}
}

//static
void LLPanelGroupVoting::impl::processGroupVoteHistoryItemReply(LLMessageSystem *msg, void**)
{
	LLUUID agent_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );
	if (gAgent.getID() != agent_id)
	{
		llwarns << "Got group voting history reply for another agent!"
			<< " Probably a userserver bug!" << llendl;
		return;
	}

	LLUUID group_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_GroupID, group_id );
	impl* self = sGroupIDs[group_id];
	if (!self) 
	{
		return;
	}

	LLUUID trans_id;
	U32	num_expected;
	
	msg->getU32Fast(_PREHASH_TransactionData,
					_PREHASH_TotalNumItems,
					num_expected);
	msg->getUUIDFast(_PREHASH_TransactionData,
					 _PREHASH_TransactionID,
					 trans_id);

	if ( self->mHistoryTransID != trans_id ) return;

	//the first item should be some text
	//explaining that we are attempting to receive information
	//or are currently receiving information, so we should clear out
	//the first line
	self->mVotesHistory->deleteAllItems();
	// make sure column indices are correct when populating with real data
	self->mVotesHistory->clearColumns();

	if ( num_expected == 0 )
	{
		//we are expecting 0 items, put up a message indicating tehre are
		//no active proposals and make the scroll list unselectable
		self->addNoHistoryScrollListItem(ADD_BOTTOM);
		self->mVotesHistory->setCanSelect(FALSE);

		return;
	}

	const S32 ITEM_NUM_MAX_BUF_LEN = 15;
	char item_num_string[ITEM_NUM_MAX_BUF_LEN];		/*Flawfinder: ignore*/
	char proposal_text[DB_VOTE_TEXT_BUF_SIZE];		/*Flawfinder: ignore*/
	char vote_result[MAX_STRING];		/*Flawfinder: ignore*/
	char start_datetime[DB_DATETIME_BUF_SIZE];		/*Flawfinder: ignore*/
	char end_datetime[DB_DATETIME_BUF_SIZE];		/*Flawfinder: ignore*/
	char vote_type[DB_VOTE_TYPE_BUF_SIZE];		/*Flawfinder: ignore*/

	LLString vote_text;
	LLUUID vote_id;
	LLUUID vote_initiator;
	LLUUID winner_id;

	F32	majority;
	S32	quorum;

	msg->getStringFast(_PREHASH_HistoryItemData, _PREHASH_TerseDateID, ITEM_NUM_MAX_BUF_LEN, item_num_string );
	msg->getUUIDFast(_PREHASH_HistoryItemData, _PREHASH_VoteID,	vote_id );
	msg->getStringFast(_PREHASH_HistoryItemData, _PREHASH_StartDateTime, DB_DATETIME_BUF_SIZE, start_datetime );
	msg->getStringFast(_PREHASH_HistoryItemData, _PREHASH_EndDateTime, DB_DATETIME_BUF_SIZE, end_datetime );
	msg->getUUIDFast(_PREHASH_HistoryItemData, _PREHASH_VoteInitiator, vote_initiator);
	msg->getStringFast(_PREHASH_HistoryItemData, _PREHASH_VoteType, DB_VOTE_TYPE_BUF_SIZE, vote_type);
	msg->getStringFast(_PREHASH_HistoryItemData,_PREHASH_VoteResult, DB_VOTE_RESULT_BUF_SIZE, vote_result);
	msg->getStringFast(_PREHASH_HistoryItemData, _PREHASH_ProposalText, DB_VOTE_TEXT_BUF_SIZE, proposal_text );
	msg->getF32Fast(_PREHASH_HistoryItemData, _PREHASH_Majority, majority);
	msg->getS32Fast(_PREHASH_HistoryItemData, _PREHASH_Quorum, quorum);

	S32 vote_items = msg->getNumberOfBlocksFast(_PREHASH_VoteItem);

	if (vote_items > 0)
	{
		if (!strcmp(vote_type, "Proposal"))
		{
			LLSD row;
			row["id"] = vote_id;

			S32 index = 0;
			row["columns"][0]["column"] = "item_num";
			row["columns"][0]["value"] = item_num_string;
			row["columns"][0]["font"] = "SANSSERIF_SMALL";
			row["columns"][0]["width"] = self->mHistoryColumnWidths[index++];
			
			vote_text.assign(proposal_text);
			vote_text.append("\n\n--\n");
			if (!strcmp(vote_result, "Success"))
			{
				vote_text.append("Proposal PASSED.");
			}
			else
			{
				vote_text.append("Proposal FAILED.");
			}
			vote_text.append("  Votes->\n");

			char vote_result[DB_VOTE_RESULT_BUF_SIZE];		/*Flawfinder: ignore*/
			char result_msg[MAX_STRING];		/*Flawfinder: ignore*/
			S32 num_votes;

			for (S32 i = 0; i < vote_items; ++i)
			{
				msg->getStringFast(_PREHASH_VoteItem, _PREHASH_VoteCast, DB_VOTE_RESULT_BUF_SIZE, vote_result, i);
				msg->getS32Fast(_PREHASH_VoteItem, _PREHASH_NumVotes, num_votes, i);
				snprintf(result_msg, MAX_STRING,			/* Flawfinder: ignore */
						"    %s: %d\n",
						vote_result,
						num_votes);
				vote_text.append(result_msg);
			}

			vote_text.append("\n");

			LLString vote_text_stripped = vote_text;
			LLString::stripNonprintable(vote_text_stripped);

			row["columns"][1]["column"] = "vote_text_stripped";
			row["columns"][1]["value"] = vote_text_stripped;
			row["columns"][1]["font"] = "SANSSERIF_SMALL";
			row["columns"][1]["width"] = self->mHistoryColumnWidths[index++];

			row["columns"][2]["column"] = "end_datetime";
			row["columns"][2]["value"] = end_datetime;
			row["columns"][2]["font"] = "SANSSERIF_SMALL";
			row["columns"][2]["width"] = self->mHistoryColumnWidths[index++];

			row["columns"][3]["column"] = "vote_type";
			row["columns"][3]["value"] = vote_type;
			row["columns"][3]["font"] = "SANSSERIF_SMALL";
			row["columns"][3]["width"] = self->mHistoryColumnWidths[index++];

			row["columns"][4]["column"] = "vote_result";
			row["columns"][4]["value"] = vote_result;
			row["columns"][4]["font"] = "SANSSERIF_SMALL";
			row["columns"][4]["width"] = self->mHistoryColumnWidths[index++];

			row["columns"][5]["column"] = "vote_text";
			row["columns"][5]["value"] = vote_text;
			row["columns"][5]["font"] = "SANSSERIF_SMALL";
			row["columns"][5]["width"] = self->mHistoryColumnWidths[index++];

			//row->addColumn(vote_text_stripped, font, self->mHistoryColumnWidths[1]);
			//row->addColumn(end_datetime, font, self->mHistoryColumnWidths[2]);
			//row->addColumn(vote_type, font, self->mHistoryColumnWidths[3]);
			//row->addColumn(vote_result, font, self->mHistoryColumnWidths[4]);
			//row->addColumn(vote_text, font, self->mHistoryColumnWidths[5]);

			self->mHistoryReceived.push_back(row);
		} //end if proposal
	} //end if vote_items > 0

	int received = self->mHistoryReceived.size();
	if ( (U32)received != num_expected )
	{
		self->addPendingHistoryScrollListItem(received,
											  num_expected,
											  ADD_BOTTOM);
		self->mVotesHistory->setCanSelect(FALSE);
	}
	else
	{
		//all done display all of the items
		std::vector<LLSD>::iterator it = self->mHistoryReceived.begin();
		std::vector<LLSD>::iterator end = self->mHistoryReceived.end();

		for (; it != end; it++)
		{
			self->mVotesHistory->addElement((*it), ADD_SORTED);
		}

		self->mVotesHistory->setCanSelect(TRUE);
	}
}

//static
void LLPanelGroupVoting::impl::onClickCreateProposal(void* userdata)
{
	impl* self = (impl*)userdata;
	if ( self ) self->setEnableCreateProposal();
}

//static
void LLPanelGroupVoting::impl::onClickYes(void *userdata)
{
	LLPanelGroupVoting::impl* self = (LLPanelGroupVoting::impl*)userdata;

	if ( self && self->mInstructions )
	{
		self->mInstructions->setText("Submitting Yes to proposal...");
		self->sendGroupProposalBallot("Yes");

		//refresh the proposals now that we've hit yes
		self->sendGroupProposalsRequest(self->mGroupID);
		self->sendGroupVoteHistoryRequest(self->mGroupID);
		self->setEnableListProposals();
	}
}

//static
void LLPanelGroupVoting::impl::onClickNo(void *userdata)
{
	LLPanelGroupVoting::impl* self = (LLPanelGroupVoting::impl*)userdata;

	if ( self && self->mInstructions )
	{
		self->mInstructions->setText("Submitting No to proposal...");
		self->sendGroupProposalBallot("No");

		//refresh the proposals now that we've hit no
		self->sendGroupProposalsRequest(self->mGroupID);
		self->sendGroupVoteHistoryRequest(self->mGroupID);
		self->setEnableListProposals();
	}
}

//static
void LLPanelGroupVoting::impl::onClickAbstain(void *userdata)
{
	impl* self = (LLPanelGroupVoting::impl*) userdata;

	if ( self && self->mInstructions )
	{
		self->mInstructions->setText("Submitting Abstention to proposal...");
		self->sendGroupProposalBallot("Abstain");

		//refresh the proposals now that we've hit abstain
		self->sendGroupProposalsRequest(self->mGroupID);
		self->sendGroupVoteHistoryRequest(self->mGroupID);
		self->setEnableListProposals();
	}
}

//static
void LLPanelGroupVoting::impl::onClickSubmitProposal(void *userdata)
{
	gFocusMgr.setKeyboardFocus(NULL, NULL);
	impl* self = (impl*)userdata;

	if ( self && self->mInstructions && self->mProposalText )
	{
		//check to see if the proposal has any text for some form of
		//"validation"
		if ( self->mProposalText->getText().empty() )
		{
			//throw up an error dialog
			LLString::format_map_t args;
			args["[MESSAGE]"] = self->mEmptyProposalText;
			gViewerWindow->alertXml("GenericAlert", args);
			return;
		}

		self->mInstructions->setText("Submitting new Proposal...");
		self->sendStartGroupProposal();

		//refresh the proposals now that we've submitted a new one
		self->sendGroupProposalsRequest(self->mGroupID);
		self->setEnableListProposals();
	}
}

//static
void LLPanelGroupVoting::impl::onClickCancelProposal(void* userdata)
{
	impl* self = (impl*)userdata;

	if ( self )
	{
		self->setEnableListProposals();
	}
}

//static
void LLPanelGroupVoting::impl::onClickViewProposalList(void *userdata)
{
	impl* self = (LLPanelGroupVoting::impl*)userdata;

	if ( self ) self->setEnableListProposals();
}

//static
void LLPanelGroupVoting::impl::onClickViewProposalItem(void *userdata)
{
	impl* self = (LLPanelGroupVoting::impl*)userdata;

	if ( self && self->mProposals )
	{
		if (self->mProposals->getFirstSelected() != NULL)
		{
			self->setEnableVoteProposal();
		}
		else
		{
			gViewerWindow->alertXml("SelectProposalToView");
		}
	}
}

//static
void LLPanelGroupVoting::impl::onDoubleClickProposal(void *userdata)
{
	impl* self = (impl*)userdata;
	if ( self && self->mProposals->getFirstSelected() != NULL )
	{
		self->setEnableVoteProposal();
	}
}

//static
void LLPanelGroupVoting::impl::onDoubleClickHistoryItem(void *userdata)
{
	impl* self = (impl*)userdata;
	if ( self && self->mVotesHistory->getFirstSelected() != NULL )
	{
		self->setEnableHistoryItem();
	}
}

//static
void LLPanelGroupVoting::impl::onClickViewHistoryItem(void *userdata)
{
	impl* self = (impl*)userdata;

	if (self->mVotesHistory->getFirstSelected() != NULL)
	{
		self->setEnableHistoryItem();
	}
	else
	{
		gViewerWindow->alertXml("SelectHistoryItemToView");
	}
}

//static
void LLPanelGroupVoting::impl::onClickViewHistoryList(void *userdata)
{
	impl* self = (impl*)userdata;

	if ( self) self->setEnableHistoryList();
}

//**********************************
//** LLPanelGroupVoting Functions **
//**********************************

// static
void* LLPanelGroupVoting::createTab(void* data)
{
	LLUUID* group_id = static_cast<LLUUID*>(data);
	return new LLPanelGroupVoting("panel group voting", *group_id);
}

LLPanelGroupVoting::LLPanelGroupVoting(const std::string& name, 
								  const LLUUID& group_id)
	: LLPanelGroupTab(name, group_id)
{
	mImpl = new impl(group_id);
}

LLPanelGroupVoting::~LLPanelGroupVoting()
{
	delete mImpl;
}

BOOL LLPanelGroupVoting::isVisibleByAgent(LLAgent* agentp)
{
	 //if they are in the group, the panel is viewable
	return mAllowEdit && agentp->isInGroup(mGroupID);
}

BOOL LLPanelGroupVoting::postBuild()
{
	bool recurse = true;

	mImpl->mQuorumText   = (LLTextBox*) getChildByName("quorum_text", recurse);
	mImpl->mDurationText = (LLTextBox*) getChildByName("duration_text",
													   recurse);
	mImpl->mQuorum       = (LLSpinCtrl*) getChildByName("quorum", recurse);
	mImpl->mQuorumLbl    = (LLTextBox*) getChildByName("quorum_lbl", recurse);
	mImpl->mDuration     = (LLSpinCtrl*) getChildByName("duration", recurse);
	mImpl->mDurationLbl  = (LLTextBox*) getChildByName("duration_lbl", recurse);
	mImpl->mMajority     = (LLRadioGroup*) getChildByName("majority", recurse);
	mImpl->mMajorityLbl  = (LLTextBox*) getChildByName("majority_lbl", recurse);
	mImpl->mInstructions = (LLTextBox*) getChildByName("proposal_instructions",
													   recurse);
	mImpl->mProposalHeader = (LLTextBox*) getChildByName("proposal_header",
														 recurse);

	mImpl->mProposalLbl  = (LLTextBox*) getChildByName("proposal_lbl", recurse);
	mImpl->mStartLbl     = (LLTextBox*) getChildByName("start_lbl", recurse);
	mImpl->mEndLbl       = (LLTextBox*) getChildByName("end_lbl", recurse);
	mImpl->mStartDate    = (LLTextBox*) getChildByName("start_date", recurse);
	mImpl->mEndDate      = (LLTextBox*) getChildByName("end_date", recurse);

	mImpl->mBtnYes     = (LLButton*) getChildByName("btn_yes", recurse);
	mImpl->mBtnNo      = (LLButton*) getChildByName("btn_no", recurse);
	mImpl->mBtnAbstain = (LLButton*) getChildByName("btn_abstain", recurse);

	mImpl->mProposals =
		(LLScrollListCtrl*) getChildByName("proposals", recurse);
	mImpl->mProposalText =
		(LLTextEditor*) getChildByName("proposal_text", recurse);

	mImpl->mBtnCreateProposal = (LLButton*) getChildByName("btn_proposal",
														   recurse);
	mImpl->mBtnSubmitProposal = (LLButton*) getChildByName("btn_submit",
														   recurse);
	mImpl->mBtnCancelProposal = (LLButton*) getChildByName("btn_cancel",
														   recurse);

	mImpl->mBtnViewProposalList =
		(LLButton*) getChildByName("btn_view_proposal_list", recurse);
	mImpl->mBtnViewProposalItem =
		(LLButton*) getChildByName("btn_view_proposal_item", recurse);

	mImpl->mVotesHistory       = (LLScrollListCtrl*)getChildByName("history_list",
																   recurse);
	mImpl->mVotesHistoryLbl    = (LLTextBox*) getChildByName("history_list_lbl",
															 recurse);
	mImpl->mVoteHistoryText    = (LLTextEditor*) getChildByName("vote_text",
																recurse);
	mImpl->mVoteHistoryTextLbl = (LLTextBox*) getChildByName("vote_text_lbl",
															 recurse);

	mImpl->mBtnViewHistoryList =
		(LLButton*)getChildByName("btn_view_history_list", recurse);
	mImpl->mBtnViewHistoryItem =
		(LLButton*)getChildByName("btn_view_history_item", recurse);

	LLTextBox *txt = (LLTextBox*) getChildByName("proposals_header_view_txt",
												 recurse);
	if (txt)
	{
		mImpl->mViewProposalHeaderText = txt->getText();
		removeChild(txt, TRUE);
	}

	txt = (LLTextBox*) getChildByName("proposals_header_create_txt", recurse);
	if (txt)
	{
		mImpl->mCreateProposalHeaderText = txt->getText();
		removeChild(txt, TRUE);
	}

	txt = (LLTextBox*) getChildByName("proposals_header_vote_txt", recurse);
	if (txt)
	{
		mImpl->mVoteProposalHeaderText = txt->getText();
		removeChild(txt, TRUE);
	}

	txt = (LLTextBox*) getChildByName("empty_proposal_txt", recurse);
	if (txt)
	{
		mImpl->mEmptyProposalText = txt->getText();
		removeChild(txt, TRUE);
	}

	mImpl->updateQuorumText();

	bool success = (mImpl->mQuorumText &&
					mImpl->mDurationText &&
					mImpl->mQuorum &&
					mImpl->mQuorumLbl &&
					mImpl->mDuration &&
					mImpl->mDurationLbl &&
					mImpl->mMajority &&
					mImpl->mMajorityLbl &&
					mImpl->mInstructions &&
					mImpl->mProposalHeader &&
					mImpl->mProposalLbl &&
					mImpl->mStartLbl &&
					mImpl->mEndLbl &&
					mImpl->mStartDate &&
					mImpl->mEndDate &&
					mImpl->mBtnYes &&
					mImpl->mBtnNo &&
					mImpl->mBtnAbstain &&
					mImpl->mProposals &&
					mImpl->mProposalText &&
					mImpl->mBtnCreateProposal &&
					mImpl->mBtnSubmitProposal &&
					mImpl->mBtnCancelProposal &&
					mImpl->mBtnViewProposalList &&
					mImpl->mBtnViewProposalItem &&
					mImpl->mVotesHistory &&
					mImpl->mVotesHistoryLbl &&
					mImpl->mVoteHistoryText &&
					mImpl->mVoteHistoryTextLbl &&
					mImpl->mBtnViewHistoryList &&
					mImpl->mBtnViewHistoryItem );

	//associate callbacks
	if ( success )
	{
		mImpl->mProposals->setDoubleClickCallback(impl::onDoubleClickProposal);
		mImpl->mProposals->setCallbackUserData(mImpl);

		mImpl->mVotesHistory->setDoubleClickCallback(impl::onDoubleClickHistoryItem);
		mImpl->mVotesHistory->setCallbackUserData(mImpl);

		mImpl->mBtnAbstain->setClickedCallback(impl::onClickAbstain);
		mImpl->mBtnAbstain->setCallbackUserData(mImpl);

		mImpl->mBtnNo->setClickedCallback(impl::onClickNo);
		mImpl->mBtnNo->setCallbackUserData(mImpl);

		mImpl->mBtnYes->setClickedCallback(impl::onClickYes);
		mImpl->mBtnYes->setCallbackUserData(mImpl);

		mImpl->mBtnCreateProposal->setClickedCallback(impl::onClickCreateProposal);
		mImpl->mBtnCreateProposal->setCallbackUserData(mImpl);

		mImpl->mBtnSubmitProposal->setClickedCallback(impl::onClickSubmitProposal);
		mImpl->mBtnSubmitProposal->setCallbackUserData(mImpl);

		mImpl->mBtnCancelProposal->setClickedCallback(impl::onClickCancelProposal);
		mImpl->mBtnCancelProposal->setCallbackUserData(mImpl);

		mImpl->mBtnViewProposalList->setClickedCallback(impl::onClickViewProposalList);
		mImpl->mBtnViewProposalList->setCallbackUserData(mImpl);

		mImpl->mBtnViewProposalItem->setClickedCallback(impl::onClickViewProposalItem);
		mImpl->mBtnViewProposalItem->setCallbackUserData(mImpl);

		mImpl->mBtnViewHistoryList->setClickedCallback(impl::onClickViewHistoryList);
		mImpl->mBtnViewHistoryList->setCallbackUserData(mImpl);

		mImpl->mBtnViewHistoryItem->setClickedCallback(impl::onClickViewHistoryItem);
		mImpl->mBtnViewHistoryItem->setCallbackUserData(mImpl);

		gMessageSystem->setHandlerFuncFast(_PREHASH_GroupActiveProposalItemReply, 
										   impl::processGroupActiveProposalItemReply);
		gMessageSystem->setHandlerFuncFast(_PREHASH_GroupVoteHistoryItemReply, 
										   impl::processGroupVoteHistoryItemReply);
	}

	return success && LLPanelGroupTab::postBuild();
}


void LLPanelGroupVoting::activate()
{
	gGroupMgr->sendGroupPropertiesRequest(mGroupID);
	update(GC_ALL);

	mImpl->setEnableListProposals();
	mImpl->setEnableHistoryList();
}

bool LLPanelGroupVoting::needsApply(LLString& mesg)
{
	return false;
}

bool LLPanelGroupVoting::apply(LLString& mesg)
{
	return true;
}

void LLPanelGroupVoting::update(LLGroupChange gc)
{
	LLGroupMgrGroupData* group_datap = NULL;
	group_datap = gGroupMgr->getGroupData(mGroupID);

	if ( group_datap )
	{
		S32 num_group_members = group_datap->mMemberCount;
		mImpl->mNumGroupMembers = num_group_members;

		mImpl->updateQuorumText();
	}

	mImpl->sendGroupProposalsRequest(mGroupID);
	mImpl->sendGroupVoteHistoryRequest(mGroupID);
}

