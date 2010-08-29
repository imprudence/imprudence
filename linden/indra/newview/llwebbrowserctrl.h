/**
 * @file llwebbrowserctrl.h
 * @brief Web browser UI control
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2009, Linden Research, Inc.
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

#ifndef LL_LLWEBBROWSERCTRL_H
#define LL_LLWEBBROWSERCTRL_H

////////////////////////////////////////////////////////////////////////////////
// data class that is passed with an event
class LLWebBrowserCtrlEvent
{
	public:
		LLWebBrowserCtrlEvent()
		{
		};

		LLWebBrowserCtrlEvent( int intValIn ) :
			mIntVal( intValIn )
		{
		};

		LLWebBrowserCtrlEvent( std::string stringValIn ) :
			mIntVal(-1),
			mStringVal( stringValIn )
		{
		};

		LLWebBrowserCtrlEvent( std::string stringValIn,  std::string stringValExIn ) :
			mIntVal(-1),
			mStringVal( stringValIn ),
			mStringValEx( stringValExIn )
		{
		};

		virtual ~LLWebBrowserCtrlEvent()
		{
		};

		int getIntValue() const
		{
			return mIntVal;
		};

		std::string getStringValue() const
		{
			return mStringVal;
		};

		std::string getStringValueEx() const
		{
			return mStringValEx;
		};

	private:
		int mIntVal;
		std::string mStringVal;
		std::string mStringValEx;
};

////////////////////////////////////////////////////////////////////////////////
// Override these methods to observe web browser control events
// (they are chained and fired after observing LLMozLibEvents)
class LLWebBrowserCtrlObserver
{
	public:
		virtual ~LLWebBrowserCtrlObserver() { };

		typedef LLWebBrowserCtrlEvent EventType;
		virtual void onNavigateBegin( const EventType& eventIn ) { };
		virtual void onNavigateComplete( const EventType& eventIn ) { };
		virtual void onUpdateProgress( const EventType& eventIn ) { };
		virtual void onStatusTextChange( const EventType& eventIn ) { };
		virtual void onLocationChange( const EventType& eventIn ) { };
		virtual void onClickLinkHref( const EventType& eventIn ) { };
		virtual void onClickLinkNoFollow( const EventType& eventIn ) { };
};

#include "lluictrl.h"
#include "llframetimer.h"
#include "lldynamictexture.h"
#include "llmediamanager.h"
#include "llmediaobserver.h"

class LLViewBorder;
class LLWebBrowserTexture;

///////////////////////////////////////////////////////////////////////////////
// manages the process of storing and emitting events that the consumer
// of the embedding class can observe
template< class T >
class LLWebBrowserCtrlEventEmitter
{
	public:
		LLWebBrowserCtrlEventEmitter() { };
		~LLWebBrowserCtrlEventEmitter() { };

		typedef typename T::EventType EventType;
		typedef std::list< T* > ObserverContainer;
		typedef void( T::*observerMethod )( const EventType& );

		///////////////////////////////////////////////////////////////////////////////
		//
		bool addObserver( T* observerIn )
		{
			if ( ! observerIn )
				return false;

			if ( std::find( observers.begin(), observers.end(), observerIn ) != observers.end() )
				return false;

			observers.push_back( observerIn );

			return true;
		};

		///////////////////////////////////////////////////////////////////////////////
		//
		bool remObserver( T* observerIn )
		{
			if ( ! observerIn )
				return false;

			observers.remove( observerIn );

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

class LLUICtrlFactory;

////////////////////////////////////////////////////////////////////////////////
//
class LLWebBrowserCtrl :
	public LLUICtrl,
	public LLMediaObserver
{
	public:
		LLWebBrowserCtrl( const std::string& name, const LLRect& rect );
		virtual ~LLWebBrowserCtrl();

		void setBorderVisible( BOOL border_visible );

		// For the tutorial window, we don't want to take focus on clicks,
		// as the examples include how to move around with the arrow
		// keys.  Thus we keep focus on the app by setting this false.
		// Defaults to true.
		void setTakeFocusOnClick( bool take_focus );

		virtual LLXMLNodePtr getXML(bool save_children = true) const;
		static LLView* fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory);

		// handle mouse related methods
		virtual BOOL handleHover( S32 x, S32 y, MASK mask );
		virtual BOOL handleMouseUp( S32 x, S32 y, MASK mask );
		virtual BOOL handleMouseDown( S32 x, S32 y, MASK mask );
		virtual BOOL handleDoubleClick( S32 x, S32 y, MASK mask );
		virtual BOOL handleScrollWheel( S32 x, S32 y, S32 clicks );

		// navigation
		void navigateTo( std::string urlIn );
		void navigateBack();
		void navigateHome();
		void navigateForward();	
		void navigateToLocalPage( const std::string& subdir, const std::string& filename_in );
		bool canNavigateBack();
		bool canNavigateForward();
		void setOpenInExternalBrowser( bool valIn );
		void setOpenInInternalBrowser( bool valIn );
		std::string getCurrentNavUrl();

		// By default, we do not handle "secondlife:///app/" SLURLs, because
		// those can cause teleports, open windows, etc.  We cannot be sure
		// that each "click" is actually due to a user action, versus 
		// Javascript or some other mechanism.  However, we need the search
		// floater and login page to handle these URLs.  Those are safe
		// because we control the page content.  See DEV-9530.  JC.
		void setTrusted( bool valIn );

		void setHomePageUrl( const std::string urlIn );
		std::string getHomePageUrl();

		// set/clear URL to visit when a 404 page is reached
		bool set404RedirectUrl( std::string redirect_url );
		bool clr404RedirectUrl();

		// accessor/mutator for flag that indicates if frequent updates to texture happen
		bool getFrequentUpdates() { return mFrequentUpdates; };
		void setFrequentUpdates( bool frequentUpdatesIn ) {  mFrequentUpdates = frequentUpdatesIn; };

		void setIgnoreUIScale(bool ignore) { mIgnoreUIScale = ignore; }
		bool getIgnoreUIScale() { return mIgnoreUIScale; }

		void setAlwaysRefresh(bool refresh) { mAlwaysRefresh = refresh; }
		bool getAlwaysRefresh() { return mAlwaysRefresh; }
		
		void setForceUpdate(bool force_update) { mForceUpdate = force_update; }
		bool getForceUpdate() { return mForceUpdate; }

		bool setCaretColor( unsigned int red, unsigned int green, unsigned int blue );


		// over-rides
		virtual BOOL handleKeyHere( KEY key, MASK mask);
		virtual BOOL handleUnicodeCharHere(llwchar uni_char);
		virtual void reshape( S32 width, S32 height, BOOL called_from_parent = TRUE);
		virtual void draw();
		virtual void onVisibilityChange ( BOOL curVisibilityIn );

		// focus overrides
		void onFocusLost();
		void onFocusReceived();

		// observer interface
		bool addObserver( LLWebBrowserCtrlObserver* subjectIn );
		bool remObserver( LLWebBrowserCtrlObserver* subjectIn );

		// LLMozlib observer overrides
		virtual void onNavigateBegin( const EventType& eventIn );
		virtual void onNavigateComplete( const EventType& eventIn );
		virtual void onUpdateProgress( const EventType& eventIn );
		virtual void onStatusTextChange( const EventType& eventIn );
		virtual void onLocationChange( const EventType& eventIn );
		virtual void onClickLinkHref( const EventType& eventIn );
		virtual void onClickLinkNoFollow( const EventType& eventIn );
		virtual void onMediaContentsChange( const EventType& event_in );
	
	protected:
		void convertInputCoords(S32& x, S32& y);

	private:
		static bool onClickLinkExternalTarget( const LLSD&, const LLSD& );

		LLWebBrowserCtrlEventEmitter< LLWebBrowserCtrlObserver > mEventEmitter;
		const S32 mTextureDepthBytes;
		int mEmbeddedBrowserWindowId;
		LLWebBrowserTexture* mWebBrowserImage;
		LLViewBorder* mBorder;
		bool mFrequentUpdates;
		bool mForceUpdate;
		bool mOpenLinksInExternalBrowser;
		bool mOpenLinksInInternalBrowser;
		bool mTrusted;
		std::string mHomePageUrl;
		std::string mExternalUrl;
		std::string mCurrentNavUrl;
		bool mIgnoreUIScale;
		bool mAlwaysRefresh;
		LLMediaBase* mMediaSource;
		bool mTakeFocusOnClick;
};

////////////////////////////////////////////////////////////////////////////////
//
class LLWebBrowserTexture : public LLDynamicTexture
{
	public:
		LLWebBrowserTexture( S32 width, S32 height, LLWebBrowserCtrl* browserCtrl, LLMediaBase *media_source );
		virtual ~LLWebBrowserTexture();

		virtual BOOL needsRender();
		virtual void preRender( BOOL clear_depth = TRUE ) {};
		virtual void postRender( BOOL success ) {};
		virtual BOOL render();
		
		S32 getBrowserWidth();
		S32 getBrowserHeight();
		void setNeedsUpdate();

		void resize( S32 new_width, S32 new_height );

	protected:
		S32 mBrowserWidth;
		S32 mBrowserHeight;
		S32 mLastBrowserDepth;
		bool mNeedsUpdate;
		LLFrameTimer mElapsedTime;
		LLWebBrowserCtrl* mWebBrowserCtrl;
		LLMediaBase *mMediaSource;
};

#endif // LL_LLWEBBROWSERCTRL_H
