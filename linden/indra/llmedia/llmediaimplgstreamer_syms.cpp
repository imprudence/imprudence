/** 
 * @file llmediaimplgstreamer_syms.cpp
 * @brief dynamic GStreamer symbol-grabbing code
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2008, Linden Research, Inc.
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

#include "linden_common.h"

#if LL_GSTREAMER_ENABLED

extern "C" {
#include <gst/gst.h>

#include <apr-1/apr_pools.h>
#include <apr-1/apr_dso.h>
}

#include "llmediaimplgstreamer.h"

#define LL_GST_SYM(REQ, GSTSYM, RTN, ...) RTN (*ll##GSTSYM)(__VA_ARGS__) = NULL
#include "llmediaimplgstreamer_syms_raw.inc"
#include "llmediaimplgstreamer_syms_rawa.inc"
#include "llmediaimplgstreamer_syms_rawv.inc"
#undef LL_GST_SYM


static bool sSymsGrabbed = false;
static apr_pool_t *sSymGSTDSOMemoryPool = NULL;
static apr_dso_handle_t *sSymGSTDSOHandleG = NULL;
static apr_dso_handle_t *sSymGSTDSOHandleV = NULL;
static apr_dso_handle_t *sSymGSTDSOHandleA = NULL;


bool grab_gst_syms(std::string gst_dso_name,
		   std::string gst_dso_name_vid,
		   std::string gst_dso_name_aud)
{
	if (sSymsGrabbed)
	{
		// already have grabbed good syms
		return TRUE;
	}

	bool sym_error = false;
	bool rtn = false;
	apr_status_t rv;
	apr_dso_handle_t *sSymGSTDSOHandle = NULL;

#define LL_GST_SYM(REQ, GSTSYM, RTN, ...) do{rv = apr_dso_sym((apr_dso_handle_sym_t*)&ll##GSTSYM, sSymGSTDSOHandle, #GSTSYM); if (rv != APR_SUCCESS) {llwarns << "Failed to grab symbol: " << #GSTSYM << llendl; if (REQ) sym_error = true;} else llinfos << "grabbed symbol: " << #GSTSYM << " from " << (void*)ll##GSTSYM << llendl;}while(0)

	//attempt to load the shared libraries
	apr_pool_create(&sSymGSTDSOMemoryPool, NULL);
  
	if ( APR_SUCCESS == (rv = apr_dso_load(&sSymGSTDSOHandle,
					       gst_dso_name.c_str(),
					       sSymGSTDSOMemoryPool) ))
	{
		llinfos << "Found DSO: " << gst_dso_name << llendl;
#include "llmediaimplgstreamer_syms_raw.inc"
      
		if ( sSymGSTDSOHandle )
		{
			sSymGSTDSOHandleG = sSymGSTDSOHandle;
			sSymGSTDSOHandle = NULL;
		}
      
		if ( APR_SUCCESS == (rv = apr_dso_load(&sSymGSTDSOHandle,
					            gst_dso_name_aud.c_str(),
						       sSymGSTDSOMemoryPool) ))
		{
			llinfos << "Found DSO: " << gst_dso_name_aud << llendl;
#include "llmediaimplgstreamer_syms_rawa.inc"
			
			if ( sSymGSTDSOHandle )
			{
				sSymGSTDSOHandleA = sSymGSTDSOHandle;
				sSymGSTDSOHandle = NULL;
			}
	  
			if ( APR_SUCCESS ==
			     (rv = apr_dso_load(&sSymGSTDSOHandle,
						gst_dso_name_vid.c_str(),
						sSymGSTDSOMemoryPool) ))
			{
				llinfos << "Found DSO: " << gst_dso_name_vid << llendl;
#include "llmediaimplgstreamer_syms_rawv.inc"
			}
			else
			{
				llwarns << "Couldn't load DSO: "
					<< gst_dso_name_vid << llendl;
				rtn = false; // failure
			}
		}
		else
		{
			llwarns << "Couldn't load DSO: "
				<< gst_dso_name_aud << llendl;
			rtn = false; // failure
		}
		
		rtn = !sym_error;
	}
	else
	{
		llwarns << "Couldn't load DSO: " << gst_dso_name << llendl;
		rtn = false; // failure
	}

	if (sym_error)
	{
		llwarns << "Failed to find necessary symbols in GStreamer libraries." << llendl;
	}
	
	if ( sSymGSTDSOHandle )
	{
		sSymGSTDSOHandleV = sSymGSTDSOHandle;
		sSymGSTDSOHandle = NULL;
	}
#undef LL_GST_SYM

	sSymsGrabbed = !!rtn;
	return rtn;
}


void ungrab_gst_syms()
{ 
	// should be safe to call regardless of whether we've
	// actually grabbed syms.

	if ( sSymGSTDSOHandleG )
	{
		apr_dso_unload(sSymGSTDSOHandleG);
		sSymGSTDSOHandleG = NULL;
	}
	
	if ( sSymGSTDSOHandleA )
	{
		apr_dso_unload(sSymGSTDSOHandleA);
		sSymGSTDSOHandleA = NULL;
	}
	
	if ( sSymGSTDSOHandleV )
	{
		apr_dso_unload(sSymGSTDSOHandleV);
		sSymGSTDSOHandleV = NULL;
	}
	
	if ( sSymGSTDSOMemoryPool )
	{
		apr_pool_destroy(sSymGSTDSOMemoryPool);
		sSymGSTDSOMemoryPool = NULL;
	}
	
	// NULL-out all of the symbols we'd grabbed
#define LL_GST_SYM(REQ, GSTSYM, RTN, ...) do{ll##GSTSYM = NULL;}while(0)
#include "llmediaimplgstreamer_syms_raw.inc"
#include "llmediaimplgstreamer_syms_rawa.inc"
#include "llmediaimplgstreamer_syms_rawv.inc"
#undef LL_GST_SYM

	sSymsGrabbed = false;
}


#endif // LL_GSTREAMER_ENABLED
