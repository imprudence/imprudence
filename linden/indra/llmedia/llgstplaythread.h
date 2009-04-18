/** 
 * @file llgstplaythread.h
 * @author Jacek Antonelli
 * @brief GStreamer playback management thread class
 *
 * $LicenseInfo:firstyear=2009&license=viewergpl$
 * 
 * Copyright (c) 2009, Jacek Antonelli
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 *
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code.  View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * $/LicenseInfo$
 */


#ifndef LL_LLGSTPLAYTHREAD_H
#define LL_LLGSTPLAYTHREAD_H

#include "linden_common.h"

#include "llthread.h"
#include "llmediaimplcommon.h"

class LLGstPlayThread: public LLThread
{
	public:

		LLGstPlayThread( LLMediaImplCommon *impl,
		                 const std::string& name, apr_pool_t *poolp );

		~LLGstPlayThread();

		virtual void run();

	private:

		// Actually, this will really only be an LLMediaImplGStreamer.
		// But we have to jump through some hoops to mutual pointer-holding.
		// There may be a better way, but I don't have the motivation to find it.
		LLMediaImplCommon *mMediaImpl;
};


#endif // LL_LLGSTPLAYTHREAD_H
