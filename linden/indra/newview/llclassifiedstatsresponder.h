/** 
 * @file llclassifiedstatsrequest.h
 * @brief Responder class for classified stats request.
 *
 * $LicenseInfo:firstyear=2006&license=internal$
 * 
 * Copyright (c) 2006-2007, Linden Research, Inc.
 * 
 * The following source code is PROPRIETARY AND CONFIDENTIAL. Use of
 * this source code is governed by the Linden Lab Source Code Disclosure
 * Agreement ("Agreement") previously entered between you and Linden
 * Lab. By accessing, using, copying, modifying or distributing this
 * software, you acknowledge that you have been informed of your
 * obligations under the Agreement and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LL_LLCLASSIFIEDSTATSRESPONDER_H
#define LL_LLCLASSIFIEDSTATSRESPONDER_H

#include "llhttpclient.h"
#include "llview.h"

class LLClassifiedStatsResponder : public LLHTTPClient::Responder
{
public:
	LLClassifiedStatsResponder(LLViewHandle classified_panel_handle);
	//If we get back a normal response, handle it here
	virtual void result(const LLSD& content);
	//If we get back an error (not found, etc...), handle it here
	virtual void error(U32 status, const std::string& reason);

protected:
	LLViewHandle mClassifiedPanelHandle;
};

#endif // LL_LLCLASSIFIEDSTATSRESPONDER_H
