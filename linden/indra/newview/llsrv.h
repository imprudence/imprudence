/** 
 * @file llsrv.h
 * @brief Wrapper for DNS SRV record lookups
 *
 * Copyright (c) 2007-2007, Linden Research, Inc.
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

#ifndef LL_LLSRV_H
#define LL_LLSRV_H

class LLSRV;

class LLSRVRecord
{
	friend class LLSRV;

protected:
	U16 mPriority;
	U16 mWeight;
	std::string mTarget;
	U16 mPort;

	LLSRVRecord(U16 priority, U16 weight, const std::string& target,
				U16 port) :
		mPriority(priority),
		mWeight(weight),
		mTarget(target),
		mPort(port) {
	}
	
public:
	U16 priority() const { return mPriority; }
	U16 weight() const { return mWeight; }
	const std::string& target() const { return mTarget; }
	U16 port() const { return mPort; }
};
	
class LLSRV
{
public:
	static std::vector<LLSRVRecord> query(const std::string& name);
	static std::vector<std::string> rewriteURI(const std::string& uri);
};

#endif // LL_LLSRV_H
