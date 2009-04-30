/**
 * @file llmediaimplfactory.cpp
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

#include "llmediaimplfactory.h"

#include <iostream>

LLMediaImplFactory* LLMediaImplFactory::sInstance = NULL;

///////////////////////////////////////////////////////////////////////////////
// static
LLMediaImplFactory* LLMediaImplFactory::getInstance()
{
	if ( ! sInstance )
		sInstance = new LLMediaImplFactory();

	return sInstance;
}

///////////////////////////////////////////////////////////////////////////////
//
void LLMediaImplFactory::registerImpl( const std::string& impl_name, LLMediaImplMakerBase* impl_maker )
{
	mNameImplMakerContainer.insert( name_impl_maker_container_t::value_type( impl_name, impl_maker ) );
}

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplMakerBase* LLMediaImplFactory::getImplMaker(	const std::string& scheme, const std::string& type )
{
	name_impl_maker_container_t::const_iterator iter;
	name_impl_maker_container_t::const_iterator begin = mNameImplMakerContainer.begin();
	name_impl_maker_container_t::const_iterator end = mNameImplMakerContainer.end();

	for(iter = begin; iter != end; ++iter)
	{
		if(( *iter->second ).supportsScheme(scheme))
		{
			return ( iter->second );
		}
	}

	for(iter = begin; iter != end; ++iter)
	{
		if(( *iter->second ).supportsMimeType(type))
		{
			return ( iter->second );
		}
	}
	int idx1 = type.find("/");
	int len = (idx1 == std::string::npos) ? 0 : idx1;
	std::string category = type.substr(0,len);
	for(iter = begin; iter != end; ++iter)
	{
		if(( *iter->second ).supportsMimeTypeCategory(category))
		{
			return ( iter->second );
		}
	}

	return NULL;
};

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplMakerBase* LLMediaImplFactory::getImplMaker( const std::string& impl_name )
{
	name_impl_maker_container_t::const_iterator found = mNameImplMakerContainer.find( impl_name );

	if ( found == mNameImplMakerContainer.end() )
	{
		return NULL;
	};

	return found->second;
}
