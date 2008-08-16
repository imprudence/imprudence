/**
 * @file llmozlib.h
 * @brief Linden Lab Mozilla wrapper.
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

#ifndef LLMOZLIB_H
#define LLMOZLIB_H

#include <string>
#include <map>

class LLEmbeddedBrowser;
class LLEmbeddedBrowserWindow;

////////////////////////////////////////////////////////////////////////////////
// data class that is passed with an event
class LLEmbeddedBrowserWindowEvent
{
	public:
		LLEmbeddedBrowserWindowEvent( int eventWindowIdIn ) :
			mEventWindowId( eventWindowIdIn )
		{
		};

		LLEmbeddedBrowserWindowEvent( int eventWindowIdIn, int intValIn ) :
			mEventWindowId( eventWindowIdIn ),
			mIntVal( intValIn )
		{
		};

		LLEmbeddedBrowserWindowEvent( int eventWindowIdIn, std::string stringValIn ) :
			mEventWindowId( eventWindowIdIn ),
			mStringVal( stringValIn )
		{
		};

		virtual ~LLEmbeddedBrowserWindowEvent()
		{
		};

		int getEventWindowId() const
		{
			return mEventWindowId;
		};

		int getIntValue() const
		{
			return mIntVal;
		};

		std::string getStringValue() const
		{
			return mStringVal;
		};

	private:
		int mEventWindowId;
		int mIntVal;
		std::string mStringVal;
};

////////////////////////////////////////////////////////////////////////////////
// Override these methods to observe browser events
class LLEmbeddedBrowserWindowObserver
{
	public:
		virtual ~LLEmbeddedBrowserWindowObserver() { };

		typedef LLEmbeddedBrowserWindowEvent EventType;
		virtual void onNavigateBegin( const EventType& eventIn ) { };
		virtual void onNavigateComplete( const EventType& eventIn ) { };
		virtual void onUpdateProgress( const EventType& eventIn ) { };
		virtual void onStatusTextChange( const EventType& eventIn ) { };
		virtual void onLocationChange( const EventType& eventIn ) { };
		virtual void onClickLinkHref( const EventType& eventIn ) { };
		virtual void onClickLinkSecondLife( const EventType& eventIn ) { };
};

////////////////////////////////////////////////////////////////////////////////
//
class LLMozLib
{
	public:
		virtual ~LLMozLib();

		// singleton access
		static LLMozLib* getInstance();

		// housekeeping
		bool init( std::string applicationDirIn, std::string componentDirIn, std::string profileDirIn );
		bool reset();
		bool setHttpProxy( bool enabledIn, std::string addressIn, int portIn );
		bool clearCache();
		bool enableCookies( bool enabledIn );
		bool clearAllCookies();
		bool enablePlugins( bool enabledIn );
		int getLastError();
		const std::string getVersion();
		void setBrowserAgentId( std::string idIn );

		// browser window
		int createBrowserWindow( void* nativeWindowHandleIn, int browserWindowWidthIn, int browserWindowHeightIn );
		bool destroyBrowserWindow( int browserWindowIdIn );
		bool setSize( int browserWindowIdIn, int widthIn, int heightIn );
		bool scrollByLines( int browserWindowIdIn, int linesIn );
		void setBackgroundColor( int browserWindowIdIn, int redIn, int greenIn, int blueIn );

		// observer interface
		bool addObserver( int browserWindowIdIn, LLEmbeddedBrowserWindowObserver* subjectIn );
		bool remObserver( int browserWindowIdIn, LLEmbeddedBrowserWindowObserver* subjectIn );

		// navigation
		bool navigateTo( int browserWindowIdIn, const std::string uriIn );
		bool navigateStop( int browserWindowIdIn );
		bool canNavigateBack( int browserWindowIdIn );
		bool navigateBack( int browserWindowIdIn );
		bool canNavigateForward( int browserWindowIdIn );
		bool navigateForward( int browserWindowIdIn );

		// access to rendered bitmap data
		const unsigned char* grabBrowserWindow( int browserWindowIdIn );
		const unsigned char* getBrowserWindowPixels( int browserWindowIdIn );
		const int getBrowserWidth( int browserWindowIdIn );
		const int getBrowserHeight( int browserWindowIdIn );
		const int getBrowserDepth( int browserWindowIdIn );
		const int getBrowserRowSpan( int browserWindowIdIn );

		// mouse/keyboard interaction
		bool mouseDown( int browserWindowIdIn, int xPosIn, int yPosIn );
		bool mouseUp( int browserWindowIdIn, int xPosIn, int yPosIn );
		bool mouseMove( int browserWindowIdIn, int xPosIn, int yPosIn );
		bool keyPress( int browserWindowIdIn, int keyCodeIn );
		bool unicodeInput ( int browserWindowIdIn, unsigned long uni_char );
		bool focusBrowser( int browserWindowIdIn, bool focusBrowserIn );

	private:
		LLMozLib();
		LLEmbeddedBrowserWindow* getBrowserWindowFromWindowId( int browserWindowIdIn );
		static LLMozLib* sInstance;
		const int mMaxBrowserWindows;
		LLEmbeddedBrowserWindow** mBrowserWindowList;
};


// Mozilla virtual keycodes.
// We don't want to suck in Mozilla headers so we copy these consts
// from nsIDOMKeyEvent.idl.

const unsigned long LL_DOM_VK_CANCEL         = 0x03;
const unsigned long LL_DOM_VK_HELP           = 0x06;
const unsigned long LL_DOM_VK_BACK_SPACE     = 0x08;
const unsigned long LL_DOM_VK_TAB            = 0x09;
const unsigned long LL_DOM_VK_CLEAR          = 0x0C;
const unsigned long LL_DOM_VK_RETURN         = 0x0D;
const unsigned long LL_DOM_VK_ENTER          = 0x0E;
const unsigned long LL_DOM_VK_SHIFT          = 0x10;
const unsigned long LL_DOM_VK_CONTROL        = 0x11;
const unsigned long LL_DOM_VK_ALT            = 0x12;
const unsigned long LL_DOM_VK_PAUSE          = 0x13;
const unsigned long LL_DOM_VK_CAPS_LOCK      = 0x14;
const unsigned long LL_DOM_VK_ESCAPE         = 0x1B;
const unsigned long LL_DOM_VK_SPACE          = 0x20;
const unsigned long LL_DOM_VK_PAGE_UP        = 0x21;
const unsigned long LL_DOM_VK_PAGE_DOWN      = 0x22;
const unsigned long LL_DOM_VK_END            = 0x23;
const unsigned long LL_DOM_VK_HOME           = 0x24;
const unsigned long LL_DOM_VK_LEFT           = 0x25;
const unsigned long LL_DOM_VK_UP             = 0x26;
const unsigned long LL_DOM_VK_RIGHT          = 0x27;
const unsigned long LL_DOM_VK_DOWN           = 0x28;
const unsigned long LL_DOM_VK_PRINTSCREEN    = 0x2C;
const unsigned long LL_DOM_VK_INSERT         = 0x2D;
const unsigned long LL_DOM_VK_DELETE         = 0x2E;

// LL_DOM_VK_0 - LL_DOM_VK_9 match their ASCII values
const unsigned long LL_DOM_VK_0              = 0x30;
const unsigned long LL_DOM_VK_1              = 0x31;
const unsigned long LL_DOM_VK_2              = 0x32;
const unsigned long LL_DOM_VK_3              = 0x33;
const unsigned long LL_DOM_VK_4              = 0x34;
const unsigned long LL_DOM_VK_5              = 0x35;
const unsigned long LL_DOM_VK_6              = 0x36;
const unsigned long LL_DOM_VK_7              = 0x37;
const unsigned long LL_DOM_VK_8              = 0x38;
const unsigned long LL_DOM_VK_9              = 0x39;

const unsigned long LL_DOM_VK_SEMICOLON      = 0x3B;
const unsigned long LL_DOM_VK_EQUALS         = 0x3D;

// LL_DOM_VK_A - LL_DOM_VK_Z match their ASCII values
const unsigned long LL_DOM_VK_A              = 0x41;
const unsigned long LL_DOM_VK_B              = 0x42;
const unsigned long LL_DOM_VK_C              = 0x43;
const unsigned long LL_DOM_VK_D              = 0x44;
const unsigned long LL_DOM_VK_E              = 0x45;
const unsigned long LL_DOM_VK_F              = 0x46;
const unsigned long LL_DOM_VK_G              = 0x47;
const unsigned long LL_DOM_VK_H              = 0x48;
const unsigned long LL_DOM_VK_I              = 0x49;
const unsigned long LL_DOM_VK_J              = 0x4A;
const unsigned long LL_DOM_VK_K              = 0x4B;
const unsigned long LL_DOM_VK_L              = 0x4C;
const unsigned long LL_DOM_VK_M              = 0x4D;
const unsigned long LL_DOM_VK_N              = 0x4E;
const unsigned long LL_DOM_VK_O              = 0x4F;
const unsigned long LL_DOM_VK_P              = 0x50;
const unsigned long LL_DOM_VK_Q              = 0x51;
const unsigned long LL_DOM_VK_R              = 0x52;
const unsigned long LL_DOM_VK_S              = 0x53;
const unsigned long LL_DOM_VK_T              = 0x54;
const unsigned long LL_DOM_VK_U              = 0x55;
const unsigned long LL_DOM_VK_V              = 0x56;
const unsigned long LL_DOM_VK_W              = 0x57;
const unsigned long LL_DOM_VK_X              = 0x58;
const unsigned long LL_DOM_VK_Y              = 0x59;
const unsigned long LL_DOM_VK_Z              = 0x5A;

const unsigned long LL_DOM_VK_CONTEXT_MENU   = 0x5D;

const unsigned long LL_DOM_VK_NUMPAD0        = 0x60;
const unsigned long LL_DOM_VK_NUMPAD1        = 0x61;
const unsigned long LL_DOM_VK_NUMPAD2        = 0x62;
const unsigned long LL_DOM_VK_NUMPAD3        = 0x63;
const unsigned long LL_DOM_VK_NUMPAD4        = 0x64;
const unsigned long LL_DOM_VK_NUMPAD5        = 0x65;
const unsigned long LL_DOM_VK_NUMPAD6        = 0x66;
const unsigned long LL_DOM_VK_NUMPAD7        = 0x67;
const unsigned long LL_DOM_VK_NUMPAD8        = 0x68;
const unsigned long LL_DOM_VK_NUMPAD9        = 0x69;
const unsigned long LL_DOM_VK_MULTIPLY       = 0x6A;
const unsigned long LL_DOM_VK_ADD            = 0x6B;
const unsigned long LL_DOM_VK_SEPARATOR      = 0x6C;
const unsigned long LL_DOM_VK_SUBTRACT       = 0x6D;
const unsigned long LL_DOM_VK_DECIMAL        = 0x6E;
const unsigned long LL_DOM_VK_DIVIDE         = 0x6F;
const unsigned long LL_DOM_VK_F1             = 0x70;
const unsigned long LL_DOM_VK_F2             = 0x71;
const unsigned long LL_DOM_VK_F3             = 0x72;
const unsigned long LL_DOM_VK_F4             = 0x73;
const unsigned long LL_DOM_VK_F5             = 0x74;
const unsigned long LL_DOM_VK_F6             = 0x75;
const unsigned long LL_DOM_VK_F7             = 0x76;
const unsigned long LL_DOM_VK_F8             = 0x77;
const unsigned long LL_DOM_VK_F9             = 0x78;
const unsigned long LL_DOM_VK_F10            = 0x79;
const unsigned long LL_DOM_VK_F11            = 0x7A;
const unsigned long LL_DOM_VK_F12            = 0x7B;
const unsigned long LL_DOM_VK_F13            = 0x7C;
const unsigned long LL_DOM_VK_F14            = 0x7D;
const unsigned long LL_DOM_VK_F15            = 0x7E;
const unsigned long LL_DOM_VK_F16            = 0x7F;
const unsigned long LL_DOM_VK_F17            = 0x80;
const unsigned long LL_DOM_VK_F18            = 0x81;
const unsigned long LL_DOM_VK_F19            = 0x82;
const unsigned long LL_DOM_VK_F20            = 0x83;
const unsigned long LL_DOM_VK_F21            = 0x84;
const unsigned long LL_DOM_VK_F22            = 0x85;
const unsigned long LL_DOM_VK_F23            = 0x86;
const unsigned long LL_DOM_VK_F24            = 0x87;

const unsigned long LL_DOM_VK_NUM_LOCK       = 0x90;
const unsigned long LL_DOM_VK_SCROLL_LOCK    = 0x91;

const unsigned long LL_DOM_VK_COMMA          = 0xBC;
const unsigned long LL_DOM_VK_PERIOD         = 0xBE;
const unsigned long LL_DOM_VK_SLASH          = 0xBF;
const unsigned long LL_DOM_VK_BACK_QUOTE     = 0xC0;
const unsigned long LL_DOM_VK_OPEN_BRACKET   = 0xDB;
const unsigned long LL_DOM_VK_BACK_SLASH     = 0xDC;
const unsigned long LL_DOM_VK_CLOSE_BRACKET  = 0xDD;
const unsigned long LL_DOM_VK_QUOTE          = 0xDE;

const unsigned long LL_DOM_VK_META           = 0xE0;

#endif // LLMOZLIB_H
