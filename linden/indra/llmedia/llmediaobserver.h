/**
 * @file llmediaobserver.h
 * @brief Derrive from this class and override methods to observe events from emitter class
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

#ifndef LLMEDIAEOBSERVER_H
#define LLMEDIAEOBSERVER_H

class LLMediaBase;

class LLMediaEvent
{
	public:
		LLMediaEvent( LLMediaBase* subject ) :
			mSubject( subject ), mIntValue(-1)
			{
			};

		LLMediaEvent( LLMediaBase* subject, std::string in ) :
			mSubject( subject ), mIntValue(-1), mStringValue(in)
			{
			};

		LLMediaEvent( LLMediaBase* subject, std::string string_in, std::string string_ex_in ) :
			mSubject( subject ), mIntValue(-1), mStringValue(string_in), mStringValueEx(string_ex_in)
			{
			};

		LLMediaEvent( LLMediaBase* subject, std::string string_in, int int_in ) :
			mSubject( subject ), mIntValue(int_in), mStringValue(string_in)
			{
			};

		LLMediaEvent( LLMediaBase* subject, int in ) :
			mSubject( subject ), mIntValue(in)
			{
			};

		virtual ~LLMediaEvent() { }

		LLMediaBase* getSubject() const
			{
				return mSubject;
			};

		int getIntValue() const
			{
				return mIntValue;
			}

		std::string getStringValue() const
			{
				return mStringValue;
			}

		std::string getStringValueEx() const
			{
				return mStringValueEx;
			}

	private:
		LLMediaBase* mSubject;
		int mIntValue;
		std::string mStringValue;
		std::string mStringValueEx;
};

class LLMediaObserver
{
	public:
		virtual ~LLMediaObserver() {}

		typedef LLMediaEvent EventType;
		virtual void onMediaPreroll( const EventType& event_in ) { }
		virtual void onMediaLoaded( const EventType& event_in ) { }
		virtual void onMediaSizeChange( const EventType& event_in ) { }
		virtual void onMediaContentsChange( const EventType& event_in ) { }
		virtual void onMediaStatusTextChange( const EventType& event_in ) { }
		virtual void onNavigateBegin( const EventType& event_in ) { }
		virtual void onNavigateComplete( const EventType& event_in ) { }
		virtual void onUpdateProgress( const EventType& event_in ) { }
		virtual void onStatusTextChange( const EventType& event_in ) { }
		virtual void onLocationChange( const EventType& event_in ) { }
		virtual void onClickLinkHref( const EventType& event_in ) { }
		virtual void onClickLinkNoFollow( const EventType& event_in ) { }
};

#endif	// LLMEDIAEOBSERVER_H
