/**
 * @file llmediaemitter.h
 * @author Callum Prentice
 * @date 2007-10-22 00:00:00
 * @brief Manages and emits events to observers
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2009, Linden Research, Inc.
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

#ifndef LLMEDIAEMITTER_H
#define LLMEDIAEMITTER_H

#include <list>
#include <algorithm>
#include <typeinfo>

///////////////////////////////////////////////////////////////////////////////
//
template< class T >
class LLMediaEmitter
{
	public:
		LLMediaEmitter() { };
		~LLMediaEmitter() { };

		typedef typename T::EventType EventType;
		typedef std::list< T* > ObserverContainer;
		typedef void( T::*observerMethod )( const EventType& );

		///////////////////////////////////////////////////////////////////////////////
		//
		bool addObserver( T* observer_in )
		{
			if ( ! observer_in )
				return false;

			if ( std::find( observers.begin(), observers.end(), observer_in) != observers.end() )
				return false;

			observers.push_back( observer_in );

			return true;
		};

		///////////////////////////////////////////////////////////////////////////////
		//
		bool remObserver( T* observer_in )
		{
			if ( ! observer_in )
				return false;

			observers.remove( observer_in );
			observers.remove( observer_in );
			observers.remove( observer_in );



			return true;
		};

		///////////////////////////////////////////////////////////////////////////////
		//
		void update( observerMethod method, const EventType& msgIn )
		{
			typename std::list< T* >::iterator iter = observers.begin();

			while( iter != observers.end() )
			{
				( ( *iter )->*method )( msgIn );

				++iter;
			};
		};

	protected:
		ObserverContainer observers;
};

#endif	// LLMEDIAEMITTER_H
