/**
 * @file llmediabase.h
 * @author Callum Prentice
 * @date 2007-10-22 00:00:00
 * @brief Abstract class that defines LLMedia public interface
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

#ifndef LLMEDIABASE_H
#define LLMEDIABASE_H

#if LL_LLMOZLIB_ENABLED && !defined ( MOZILLA_INTERNAL_API )
	// Without this, nsTAString.h errors out with:
	// "Cannot use internal string classes without MOZILLA_INTERNAL_API defined. Use the frozen header nsStringAPI.h instead."
	// It might be worth our while to figure out if we can use the frozen apis at some point...
	#define MOZILLA_INTERNAL_API 1
#endif

#include <string>

class LLMediaObserver;
class LLMediaImplMakerBase;

class LLMediaBase
{
	public:
		LLMediaBase() {};
		virtual ~LLMediaBase() {};

		////////////////////////////////////////////////////////////////////////////////
		// housekeeping

		// local initialization, called by the media manager when creating a source
		virtual bool init() = 0;

		// undoes everything init() didm called by the media manager when destroying a source
		virtual bool reset() = 0;

		// accessor for MIME type
		virtual bool setMimeType( const std::string mime_type ) = 0;
		virtual std::string getMimeType() const = 0;

		// accessor for intial URL.  Note that this may have changed under the hood
		// so pass back the original URL seeded to this impl
		virtual std::string getMediaURL() const = 0;

		// ask impl for version string
		virtual std::string getVersion() = 0;

		// set/clear URL to visit when a 404 page is reached
		virtual bool set404RedirectUrl( std::string redirect_url ) = 0;
		virtual bool clr404RedirectUrl() = 0;

		// sets the background color of the browser window
		virtual bool setBackgroundColor( unsigned int red, unsigned int green, unsigned int blue ) const = 0;

		// sets the color of the caret in media impls that have one
		virtual bool setCaretColor( unsigned int red, unsigned int green, unsigned int blue ) const = 0;

		////////////////////////////////////////////////////////////////////////////////
		// media management

		// needs to be called regularly to make media stream update itself
		virtual bool updateMedia() = 0;

		// allows you to request a change in media width, height - may fail if media doesn't support size change
		virtual bool setRequestedMediaSize( int media_width, int media_height ) = 0;

		// gets media width (may change throughout lifetime of media stream) - event emitted when media size changed too
		virtual int getMediaWidth() const = 0;

		// gets media height (may change throughout lifetime of media stream) - event emitted when media size changed too
		virtual int getMediaHeight() const = 0;

		// allows you to try to explicitly change media depth - may fail if media doesn't support depth change
		virtual bool setMediaDepth( int media_depth ) = 0;

		// gets media depth (may change throughout lifetime of media stream) - event emitted when media depth changed too
		virtual int getMediaDepth() const = 0;

		// gets size of media buffer for current frame (might NOT be the same as media width * height * depth)
		virtual int getMediaBufferSize() const = 0;

		// returns pointer to raw media pixels
		virtual unsigned char* getMediaData() = 0;

		// returns the size of the data, which may be different that the size of the media
		virtual int getMediaDataWidth() const = 0;
		virtual int getMediaDataHeight() const = 0;

		////////////////////////////////////////////////////////////////////////////////
		// texture management

		// gets internal format to use for OpenGL texture
		virtual int getTextureFormatInternal() const = 0;

		// gets primary format to use for OpenGL texture
		virtual int getTextureFormatPrimary() const = 0;

		// gets format type to use for OpenGL texture
		virtual int getTextureFormatType() const = 0;




		////////////////////////////////////////////////////////////////////////////////
		// audio

		// set/get control volume from media stream if present
		virtual bool setVolume( float volume ) = 0;
		virtual float getVolume() const = 0;


		////////////////////////////////////////////////////////////////////////////////
		// transport control etc.
		enum ECommand {
			COMMAND_NONE	= 0,
			COMMAND_STOP	= 1,
			COMMAND_START	= 2,
			COMMAND_PAUSE	= 4,
			COMMAND_BACK	= 5,
			COMMAND_FORWARD	= 6
		};
		enum EStatus {
			STATUS_UNKNOWN		= 0,
			STATUS_INITIALIZING	= 1,
			STATUS_NAVIGATING	= 2,
			STATUS_STARTED		= 3,
			STATUS_STOPPED		= 4,
			STATUS_PAUSED		= 6,
			STATUS_RESETTING	= 7
		};
		virtual bool addCommand( ECommand cmd ) = 0;
		virtual bool clearCommand() = 0;
		virtual bool updateCommand() = 0;
		virtual EStatus getStatus() = 0;
		virtual bool seek( double time ) = 0;
		virtual bool setLooping( bool enable) = 0;
		virtual bool isLooping() = 0;

		////////////////////////////////////////////////////////////////////////////////
		// scaling

		// autoscale means try to scale media to size of texture - may fail if media doesn't support size change
		virtual bool setAutoScaled( bool auto_scaled ) = 0;
		virtual bool isAutoScaled() const = 0;


		////////////////////////////////////////////////////////////////////////////////
		// mouse and keyboard interaction
		virtual bool mouseDown( int x_pos, int y_pos ) = 0;
		virtual bool mouseUp( int x_pos, int y_pos ) = 0;
		virtual bool mouseMove( int x_pos, int y_pos ) = 0;
		virtual bool keyPress( int key_code ) = 0;
		virtual bool scrollByLines( int lines ) = 0;
		virtual bool focus( bool focus ) = 0;
		virtual bool unicodeInput( unsigned long uni_char ) = 0;
		virtual bool mouseLeftDoubleClick( int x_pos, int y_pos ) = 0;

		
		////////////////////////////////////////////////////////////////////////////////
		// navigation
		virtual bool navigateTo( const std::string url ) = 0;
		virtual bool navigateForward() = 0;
		virtual bool navigateBack() = 0;
		virtual bool canNavigateForward() = 0;
		virtual bool canNavigateBack() = 0;
		
		////////////////////////////////////////////////////////////////////////////////
		// caching/cookies
		virtual bool enableCookies( bool enable ) = 0;
		virtual bool clearCache() = 0;
		virtual bool clearCookies() = 0;

		////////////////////////////////////////////////////////////////////////////////
		// proxy
		virtual bool enableProxy(bool enable, std::string proxy_host_name, int proxy_port) = 0;

		////////////////////////////////////////////////////////////////////////////////
		// observer interface
		virtual bool addObserver( LLMediaObserver* subject ) = 0;
		virtual bool remObserver( LLMediaObserver* subject ) = 0;

		////////////////////////////////////////////////////////////////////////////////
		// factory interface
		virtual void setImplMaker(LLMediaImplMakerBase* impl_maker) = 0;

		////////////////////////////////////////////////////////////////////////////////
		// type registry interface
		virtual bool supportsMediaType(std::string scheme, std::string type) = 0;
};

//////////////////////////////////////////////////////////////
// media key codes - (mirroring mozilla's values)
const unsigned long LL_MEDIA_KEY_BACKSPACE             = 0x08;
const unsigned long LL_MEDIA_KEY_TAB                   = 0x09;
const unsigned long LL_MEDIA_KEY_RETURN                = 0x0D;
const unsigned long LL_MEDIA_KEY_PAD_RETURN            = 0x0E;
const unsigned long LL_MEDIA_KEY_ESCAPE                = 0x1B;
const unsigned long LL_MEDIA_KEY_PAGE_UP               = 0x21;
const unsigned long LL_MEDIA_KEY_PAGE_DOWN             = 0x22;
const unsigned long LL_MEDIA_KEY_END                   = 0x23;
const unsigned long LL_MEDIA_KEY_HOME                  = 0x24;
const unsigned long LL_MEDIA_KEY_LEFT                  = 0x25;
const unsigned long LL_MEDIA_KEY_UP                    = 0x26;
const unsigned long LL_MEDIA_KEY_RIGHT                 = 0x27;
const unsigned long LL_MEDIA_KEY_DOWN                  = 0x28;
const unsigned long LL_MEDIA_KEY_INSERT                = 0x2D;
const unsigned long LL_MEDIA_KEY_DELETE                = 0x2E;

//////////////////////////////////////////////////////////////
// media frame buffer types - (mirroring GL values)
const int LL_MEDIA_UNSIGNED_BYTE                      = 0x1401;
const int LL_MEDIA_RGB                                = 0x1907;
const int LL_MEDIA_RGBA                               = 0x1908;
const int LL_MEDIA_RGB8                               = 0x8051;
const int LL_MEDIA_UNSIGNED_INT_8_8_8_8               = 0x8035;
const int LL_MEDIA_UNSIGNED_INT_8_8_8_8_REV           = 0x8367;
const int LL_MEDIA_BGR                                = 0x80E0;
const int LL_MEDIA_BGRA                               = 0x80E1;


#endif	// LLMEDIABASE_H
