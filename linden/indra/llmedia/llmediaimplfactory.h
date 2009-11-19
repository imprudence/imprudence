/**
 * @file llmediaimplfactory.h
 * @brief Creates media impls that have registered themselves with LLMediaRegster
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
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

#ifndef LLMEDIAIMPLFACTORY_H
#define LLMEDIAIMPLFACTORY_H

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "llmediabase.h"

///////////////////////////////////////////////////////////////////////////////
//
class LLMediaImplMakerBase
{
	public:
		virtual bool supportsScheme(std::string scheme) = 0;
		virtual bool supportsMimeType(std::string type) = 0;
		virtual bool supportsMimeTypeCategory(std::string category) = 0;
		virtual LLMediaBase* create() = 0;
		virtual ~LLMediaImplMakerBase() {};

	protected:
		typedef std::vector <std::string> vector_impl_registry_t;
		vector_impl_registry_t mSchema;
		vector_impl_registry_t mMimeTypes;
		vector_impl_registry_t mMimeTypeCategories;
};

///////////////////////////////////////////////////////////////////////////////
//
class LLMediaImplMaker : public LLMediaImplMakerBase
{
	public:
		bool supportsScheme(std::string scheme)
		{
			vector_impl_registry_t::iterator found = std::find(mSchema.begin(), mSchema.end(), scheme);
			return found != mSchema.end();
		}
		bool supportsMimeType(std::string type)
		{
			vector_impl_registry_t::iterator found = std::find(mMimeTypes.begin(), mMimeTypes.end(), type);
			return found != mMimeTypes.end();
		}
		bool supportsMimeTypeCategory(std::string category)
		{
			vector_impl_registry_t::iterator found = std::find(mMimeTypeCategories.begin(), mMimeTypeCategories.end(), category);
			return found != mMimeTypeCategories.end();
		}
};

///////////////////////////////////////////////////////////////////////////////
//
class LLMediaImplFactory
{
	public:
		static LLMediaImplFactory* getInstance();
		void registerImpl( const std::string& impl_name, LLMediaImplMakerBase* impl_maker );
		LLMediaImplMakerBase* getImplMaker( const std::string& scheme, const std::string& type );
		LLMediaImplMakerBase* getImplMaker( const std::string& impl_name);

	private:
		typedef std::map< std::string, LLMediaImplMakerBase* > name_impl_maker_container_t;
		name_impl_maker_container_t mNameImplMakerContainer;

		static LLMediaImplFactory* sInstance;
};

#endif	// LLMEDIAIMPLFACTORY_H
