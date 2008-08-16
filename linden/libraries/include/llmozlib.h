////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
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
		bool init( std::string appBaseDirIn, std::string profileDirIn );
		bool reset();
		bool clearCache();
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
		bool focusBrowser( int browserWindowIdIn, bool focusBrowserIn );

	private:
		LLMozLib();
		LLEmbeddedBrowserWindow* getBrowserWindowFromWindowId( int browserWindowIdIn );
		static LLMozLib* sInstance;
		const int mMaxBrowserWindows;
		LLEmbeddedBrowserWindow** mBrowserWindowList;
};

#endif // LLMOZLIB_H