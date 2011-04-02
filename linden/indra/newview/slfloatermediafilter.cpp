/** 
 * @file slfloatermediafilter.cpp
 * @brief The SLFloaterMediaFilter class definitions
 *
 * $LicenseInfo:firstyear=2011&license=viewergpl$
 * 
 * Copyright (c) 2011, Sione Lomu
 * with debugging and improvements by Henri Beauchamp
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

#include "llviewerprecompiledheaders.h"

#include "lllineeditor.h"
#include "llscrolllistctrl.h"
#include "lluictrlfactory.h" 

#include "slfloatermediafilter.h"
#include "llviewercontrol.h"
#include "llviewerparcelmedia.h"

SLFloaterMediaFilter* SLFloaterMediaFilter::sInstance = NULL;
bool SLFloaterMediaFilter::sIsWhitelist = false;
bool SLFloaterMediaFilter::sIsAdding = false;
bool SLFloaterMediaFilter::sShowIPs = false;

SLFloaterMediaFilter::SLFloaterMediaFilter() : LLFloater(std::string("media filter")), mIsDirty(false)
{
    LLUICtrlFactory::getInstance()->buildFloater(this, "floater_media_filter.xml");
}

SLFloaterMediaFilter::~SLFloaterMediaFilter()
{
    sInstance = NULL;
}

BOOL SLFloaterMediaFilter::postBuild()
{
	mWhitelistSLC = getChild<LLScrollListCtrl>("whitelist_list");
	mBlacklistSLC = getChild<LLScrollListCtrl>("blacklist_list");

	if (mWhitelistSLC && mBlacklistSLC)
	{
		childSetAction("clear_lists", onClearLists, this);
		childSetAction("show_ips", onShowIPs, this);
		childSetAction("add_whitelist", onWhitelistAdd, this);
		childSetAction("remove_whitelist", onWhitelistRemove, this);
		childSetAction("add_blacklist", onBlacklistAdd, this);
		childSetAction("remove_blacklist", onBlacklistRemove, this);
		childSetAction("commit_domain", onCommitDomain, this);
		childSetAction("cancel_domain", onCancelDomain, this);
		childSetUserData("whitelist_list", this);
		childSetUserData("blacklist_list", this);
		mIsDirty = true;
	}
	gSavedSettings.getControl("MediaEnableFilter")->getSignal()->connect(boost::bind(&onMediaEnableFilterChanged, _1));

	return TRUE;
}

void SLFloaterMediaFilter::draw()
{
	if (mIsDirty && mWhitelistSLC && mBlacklistSLC)
	{
		S32 whitescrollpos = mWhitelistSLC->getScrollPos();
		S32 blackscrollpos = mBlacklistSLC->getScrollPos();
		mWhitelistSLC->deleteAllItems();
		mBlacklistSLC->deleteAllItems();
		std::set<std::string> listed;
		LLHost host;
		std::string ip;
		std::string domain;
		std::string action;
		LLSD element;
		element["columns"][0]["font"] = "SANSSERIF";
		element["columns"][0]["font-style"] = "BOLD";
		for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
		{
			domain = LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString();
			if (sShowIPs)
			{
				host.setHostByName(domain);
				ip = host.getIPString();
				if (ip != domain && domain.find('/') == std::string::npos)
				{
					domain += " (" + ip + ")";
				}
			}

			action = LLViewerParcelMedia::sMediaFilterList[i]["action"].asString();
			if (!domain.empty() && action == "allow")
			{	
				element["columns"][0]["column"] = "whitelist_col";
				element["columns"][0]["value"] = domain;
				//element["columns"][0]["color"] = LLColor4::green3.getValue();
				mWhitelistSLC->addElement(element, ADD_BOTTOM);
				listed.insert(domain);
			}
			else if (!domain.empty() && action == "deny")
			{
				element["columns"][0]["column"] = "blacklist_col";
				element["columns"][0]["value"] = domain;
				//element["columns"][0]["color"] = LLColor4::red2.getValue();
				mBlacklistSLC->addElement(element, ADD_BOTTOM);
				listed.insert(domain);
			}
			else
			{
				LL_WARNS("MediaFilter") << "Bad media filter list: removing corrupted entry for \"" << domain << "\"" << LL_ENDL;
				LLViewerParcelMedia::sMediaFilterList.erase(i--);
			}
		}
		std::set<std::string>::iterator it;
		element["columns"][0]["font"] = "SANSSERIF";
		element["columns"][0]["font-style"] = "ITALIC";
		//element["columns"][0]["color"] = LLColor4::green3.getValue();
		element["columns"][0]["column"] = "whitelist_col";
		for (it = LLViewerParcelMedia::sAllowedMedia.begin(); it != LLViewerParcelMedia::sAllowedMedia.end(); it++)
		{
			domain = *it;
			if (sShowIPs)
			{
				host.setHostByName(domain);
				ip = host.getIPString();
				if (ip != domain && domain.find('/') == std::string::npos)
				{
					domain += " (" + ip + ")";
				}
			}
			if (listed.count(domain) == 0)
			{
				element["columns"][0]["value"] = domain;
				mWhitelistSLC->addElement(element, ADD_BOTTOM);
			}
		}
		element["columns"][0]["column"] = "blacklist_col";
		for (it = LLViewerParcelMedia::sDeniedMedia.begin(); it != LLViewerParcelMedia::sDeniedMedia.end(); it++)
		{
			domain = *it;
			if (sShowIPs)
			{
				host.setHostByName(domain);
				ip = host.getIPString();
				if (ip != domain && domain.find('/') == std::string::npos)
				{
					domain += " (" + ip + ")";
				}
			}
			if (listed.count(domain) == 0)
			{
				element["columns"][0]["value"] = domain;
				mBlacklistSLC->addElement(element, ADD_BOTTOM);
			}
		}
		mWhitelistSLC->setScrollPos(whitescrollpos);
		mBlacklistSLC->setScrollPos(blackscrollpos);

		updateWidgets();

		mIsDirty = false;
		sShowIPs = false;
	}

	LLFloater::draw();
}

void SLFloaterMediaFilter::updateWidgets()
{
	bool enabled = gSavedSettings.getBOOL("MediaEnableFilter");
	childSetEnabled("clear_lists", enabled && !sIsAdding);
	childSetEnabled("show_ips", enabled && !sIsAdding);
	childSetEnabled("blacklist_list", enabled && !sIsAdding);
	childSetEnabled("whitelist_list", enabled && !sIsAdding);
	childSetEnabled("remove_whitelist", enabled && !sIsAdding);
	childSetEnabled("add_whitelist", enabled && !sIsAdding);
	childSetEnabled("remove_blacklist", enabled && !sIsAdding);
	childSetEnabled("add_blacklist", enabled && !sIsAdding);
	childSetEnabled("match_ip", enabled);
	childSetEnabled("input_domain", enabled && sIsAdding);
	childSetEnabled("commit_domain", enabled && sIsAdding);
	childSetEnabled("cancel_domain", enabled && sIsAdding);
	childSetEnabled("filter_only_remote", enabled);
	childSetEnabled("info1", enabled);
	childSetEnabled("info2", enabled);

	std::string add_text("****** Media filtering is currently DISABLED ******");
	if (enabled)
	{
		if (sIsAdding)
		{
			if (sIsWhitelist)
			{
				add_text = "Enter the domain/url to add to the white list:";
			}
			else
			{
				add_text = "Enter the domain/url to add to the black list:";
			}
		}
		else
		{
			add_text = "New Domain:";
		}
	}
	childSetText("add_text", add_text);
}

void SLFloaterMediaFilter::setDirty()
{
    if (sInstance)
	{
		sInstance->mIsDirty = true;
		sInstance->draw();
	}
}

BOOL SLFloaterMediaFilter::instanceVisible()
{
	if (sInstance)
	{
		return sInstance->getVisible();
	}
	else
	{
		return FALSE;
	}
}

void SLFloaterMediaFilter::toggleInstance()
{
	if (sInstance)
	{
		if (sInstance->getVisible())
		{
			sInstance->destroy();
		}
		else
		{
			sInstance->open();
		}
	}
	else
	{
		sInstance = new SLFloaterMediaFilter();
    	sInstance->open();
	}
}

void SLFloaterMediaFilter::onClearLists(void* data)
{
	LLViewerParcelMedia::clearDomainFilterList();
}

void SLFloaterMediaFilter::onShowIPs(void* data)
{
	sShowIPs = true;
	setDirty();
}

void SLFloaterMediaFilter::onWhitelistAdd(void* data)
{
    if (!sInstance)
	{
		return;
	}
	sIsWhitelist = true;
	sIsAdding = true;
	sInstance->updateWidgets();
}

void SLFloaterMediaFilter::onWhitelistRemove(void* data)
{
    if (!sInstance)
	{
		return;
	}
	LLScrollListItem* selected = sInstance->mWhitelistSLC->getFirstSelected();

	if (selected)
	{
		std::string domain = sInstance->mWhitelistSLC->getSelectedItemLabel();
		size_t pos = domain.find(' ');
		if (pos != std::string::npos)
		{
			domain = domain.substr(0, pos);
		}

		LLViewerParcelMedia::sAllowedMedia.erase(domain);

		for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
		{
			if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == domain)
			{
				LLViewerParcelMedia::sMediaFilterList.erase(i);
				break;
			}
		}

		if (sInstance->childGetValue("match_ip") && domain.find('/') == std::string::npos)
		{
			LLHost host;
			host.setHostByName(domain);
			std::string ip = host.getIPString();

			if (ip != domain)
			{
				LLViewerParcelMedia::sAllowedMedia.erase(ip);

				for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
				{
					if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == ip)
					{
						LLViewerParcelMedia::sMediaFilterList.erase(i);
						break;
					}
				}
			}
		}

		LLViewerParcelMedia::saveDomainFilterList();
		setDirty();
	}
}

void SLFloaterMediaFilter::onBlacklistAdd(void* data)
{
    if (!sInstance)
	{
		return;
	}
	sIsWhitelist = false;
	sIsAdding = true;
	sInstance->updateWidgets();
}

void SLFloaterMediaFilter::onBlacklistRemove(void* data)
{	
    if (!sInstance)
	{
		return;
	}
	LLScrollListItem* selected = sInstance->mBlacklistSLC->getFirstSelected();

	if (selected)
	{
		std::string domain = sInstance->mBlacklistSLC->getSelectedItemLabel();
		size_t pos = domain.find(' ');
		if (pos != std::string::npos)
		{
			domain = domain.substr(0, pos);
		}

		LLViewerParcelMedia::sDeniedMedia.erase(domain);

		for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
		{
			if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == domain)
			{
				LLViewerParcelMedia::sMediaFilterList.erase(i);
				break;
			}
		}

		if (sInstance->childGetValue("match_ip") && domain.find('/') == std::string::npos)
		{
			LLHost host;
			host.setHostByName(domain);
			std::string ip = host.getIPString();

			if (ip != domain)
			{
				LLViewerParcelMedia::sDeniedMedia.erase(ip);

				for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
				{
					if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == ip)
					{
						LLViewerParcelMedia::sMediaFilterList.erase(i);
						break;
					}
				}
			}
		}

		LLViewerParcelMedia::saveDomainFilterList();
		setDirty();
	}
}	

void SLFloaterMediaFilter::onCommitDomain(void* data)
{
    if (!sInstance)
	{
		return;
	}
	std::string domain = sInstance->childGetText("input_domain");
	domain = LLViewerParcelMedia::extractDomain(domain);
	LLHost host;
	host.setHostByName(domain);
	std::string ip = host.getIPString();
	bool match_ip = (sInstance->childGetValue("match_ip") && ip != domain && domain.find('/') == std::string::npos);

	if (!domain.empty())
	{
		LLViewerParcelMedia::sDeniedMedia.erase(domain);
		LLViewerParcelMedia::sAllowedMedia.erase(domain);
		for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
		{
			if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == domain)
			{
				LLViewerParcelMedia::sMediaFilterList.erase(i);
			}
		}
		if (match_ip)
		{
			LLViewerParcelMedia::sDeniedMedia.erase(ip);
			LLViewerParcelMedia::sAllowedMedia.erase(ip);
			for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
			{
				if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == ip)
				{
					LLViewerParcelMedia::sMediaFilterList.erase(i);
				}
			}
		}
		LLSD newmedia;
		newmedia["domain"] = domain;
		if (sIsWhitelist)
		{
			newmedia["action"] = "allow";
		}
		else
		{
			newmedia["action"] = "deny";
		}
		LLViewerParcelMedia::sMediaFilterList.append(newmedia);
		if (match_ip)
		{
			newmedia["domain"] = ip;
			LLViewerParcelMedia::sMediaFilterList.append(newmedia);
		}
		LLViewerParcelMedia::saveDomainFilterList();
	}

	sInstance->childSetText("input_domain", std::string(""));
	sIsAdding = false;
	setDirty();
}

//static
void SLFloaterMediaFilter::onCancelDomain(void* data)
{
	if (!sInstance)
	{
		return;
	}
	sInstance->childSetText("input_domain", std::string(""));
	sIsAdding = false;
	setDirty();
}

//static
void SLFloaterMediaFilter::onMediaEnableFilterChanged(const LLSD& newvalue)
{
	setDirty();
}
