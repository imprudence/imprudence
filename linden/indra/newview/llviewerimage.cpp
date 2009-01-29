/** 
 * @file llviewerimage.cpp
 * @brief Object which handles a received image (and associated texture(s))
 *
 * $LicenseInfo:firstyear=2000&license=viewergpl$
 * 
 * Copyright (c) 2000-2009, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llviewerimage.h"

// Library includes
#include "imageids.h"
#include "llmath.h"
#include "llerror.h"
#include "llgl.h"
#include "llglheaders.h"
#include "llhost.h"
#include "llimage.h"
#include "llimagebmp.h"
#include "llimagej2c.h"
#include "llimagetga.h"
#include "llmemtype.h"
#include "llstl.h"
#include "llvfile.h"
#include "llvfs.h"
#include "message.h"
#include "lltimer.h"

// viewer includes
#include "lldrawpool.h"
#include "lltexturefetch.h"
#include "llviewerimagelist.h"
#include "llviewercontrol.h"
#include "pipeline.h"
#include "llappviewer.h"
///////////////////////////////////////////////////////////////////////////////

// statics
LLPointer<LLViewerImage> LLViewerImage::sMissingAssetImagep = NULL;
LLPointer<LLViewerImage> LLViewerImage::sWhiteImagep = NULL;
LLPointer<LLImageGL> LLViewerImage::sDefaultImagep = NULL;
LLPointer<LLViewerImage> LLViewerImage::sSmokeImagep = NULL;
LLPointer<LLImageGL> LLViewerImage::sNullImagep = NULL;

S32 LLViewerImage::sImageCount = 0;
S32 LLViewerImage::sRawCount = 0;
S32 LLViewerImage::sAuxCount = 0;
LLTimer LLViewerImage::sEvaluationTimer;
F32 LLViewerImage::sDesiredDiscardBias = 0.f;
static F32 sDesiredDiscardBiasMin = -2.0f; // -max number of levels to improve image quality by
static F32 sDesiredDiscardBiasMax = 1.5f; // max number of levels to reduce image quality by
F32 LLViewerImage::sDesiredDiscardScale = 1.1f;
S32 LLViewerImage::sBoundTextureMemory = 0;
S32 LLViewerImage::sTotalTextureMemory = 0;
S32 LLViewerImage::sMaxBoundTextureMem = 0;
S32 LLViewerImage::sMaxTotalTextureMem = 0;
BOOL LLViewerImage::sDontLoadVolumeTextures = FALSE;

// static
void LLViewerImage::initClass()
{
	sNullImagep = new LLImageGL(1,1,3,TRUE);
	LLPointer<LLImageRaw> raw = new LLImageRaw(1,1,3);
	raw->clear(0x77, 0x77, 0x77, 0xFF);
	sNullImagep->createGLTexture(0, raw);

#if 1
	LLPointer<LLViewerImage> imagep = new LLViewerImage(IMG_DEFAULT, TRUE);
	sDefaultImagep = imagep;
	const S32 dim = 128;
	LLPointer<LLImageRaw> image_raw = new LLImageRaw(dim,dim,3);
	U8* data = image_raw->getData();
	for (S32 i = 0; i<dim; i++)
	{
		for (S32 j = 0; j<dim; j++)
		{
#if 0
			const S32 border = 2;
			if (i<border || j<border || i>=(dim-border) || j>=(dim-border))
			{
				*data++ = 0xff;
				*data++ = 0xff;
				*data++ = 0xff;
			}
			else
#endif
			{
				*data++ = 0x7f;
				*data++ = 0x7f;
				*data++ = 0x7f;
			}
		}
	}
	imagep->createGLTexture(0, image_raw);
	image_raw = NULL;
	gImageList.addImage(imagep);
	imagep->dontDiscard();
#else
 	sDefaultImagep = gImageList.getImage(IMG_DEFAULT, TRUE, TRUE);
#endif
 	sSmokeImagep = gImageList.getImage(IMG_SMOKE, TRUE, TRUE);

}

// static
void LLViewerImage::cleanupClass()
{
	stop_glerror();
	sNullImagep = NULL;
	sDefaultImagep = NULL;
	sSmokeImagep = NULL;
	sMissingAssetImagep = NULL;
	sWhiteImagep = NULL;
}

// tuning params
const F32 discard_bias_delta = .05f;
const F32 discard_delta_time = 0.5f;
const S32 min_non_tex_system_mem = (128<<20); // 128 MB
// non-const (used externally
F32 texmem_lower_bound_scale = 0.85f;
F32 texmem_middle_bound_scale = 0.925f;

//static
void LLViewerImage::updateClass(const F32 velocity, const F32 angular_velocity)
{
	sBoundTextureMemory = LLImageGL::sBoundTextureMemory;//in bytes
	sTotalTextureMemory = LLImageGL::sGlobalTextureMemory;//in bytes
	sMaxBoundTextureMem = gImageList.getMaxResidentTexMem();//in MB	
	sMaxTotalTextureMem = gImageList.getMaxTotalTextureMem() ;//in MB

	if ((sBoundTextureMemory >> 20) >= sMaxBoundTextureMem ||
		(sTotalTextureMemory >> 20) >= sMaxTotalTextureMem)
	{
		// If we are using more texture memory than we should,
		// scale up the desired discard level
		if (sEvaluationTimer.getElapsedTimeF32() > discard_delta_time)
		{
			sDesiredDiscardBias += discard_bias_delta;
			sEvaluationTimer.reset();
		}
	}
	else if (sDesiredDiscardBias > 0.0f &&
			 (sBoundTextureMemory >> 20) < sMaxBoundTextureMem*texmem_lower_bound_scale &&
			 (sTotalTextureMemory >> 20) < sMaxTotalTextureMem*texmem_lower_bound_scale)
	{
		// If we are using less texture memory than we should,
		// scale down the desired discard level
		if (sEvaluationTimer.getElapsedTimeF32() > discard_delta_time)
		{
			sDesiredDiscardBias -= discard_bias_delta;
			sEvaluationTimer.reset();
		}
	}
	sDesiredDiscardBias = llclamp(sDesiredDiscardBias, sDesiredDiscardBiasMin, sDesiredDiscardBiasMax);
}

// static
LLViewerImage* LLViewerImage::getImage(const LLUUID& image_id)
{
	return gImageList.getImage(image_id);
}

//----------------------------------------------------------------------------

const U32 LLViewerImage::sCurrentFileVersion = 1;

LLViewerImage::LLViewerImage(const LLUUID& id, BOOL usemipmaps)
	: LLImageGL(usemipmaps),
	  mID(id)
{
	init(true);
	sImageCount++;
}

LLViewerImage::LLViewerImage(const std::string& filename, const LLUUID& id, BOOL usemipmaps)
	: LLImageGL(usemipmaps),
	  mID(id),
	  mLocalFileName(filename)
{
	init(true);
	sImageCount++;
}


LLViewerImage::LLViewerImage(const U32 width, const U32 height, const U8 components, BOOL usemipmaps)
	: LLImageGL(width, height, components, usemipmaps)
{
	init(true);
	// Create an empty image of the specified size and width
	mID.generate();
	mFullyLoaded = TRUE;
	sImageCount++;
}

LLViewerImage::LLViewerImage(const LLImageRaw* raw, BOOL usemipmaps)
	: LLImageGL(raw, usemipmaps)
{
	init(true);
	// Create an empty image of the specified size and width
	mID.generate();
	mFullyLoaded = TRUE;
	sImageCount++;
}

void LLViewerImage::init(bool firstinit)
{
	mFullWidth = 0;
	mFullHeight = 0;
	mOrigWidth = 0;
	mOrigHeight = 0;
	mNeedsAux = FALSE;
	mTexelsPerImage = 64.f*64.f;
	mMaxVirtualSize = 0.f;
	mDiscardVirtualSize = 0.f;
	mRequestedDiscardLevel = -1;
	mRequestedDownloadPriority = 0.f;
	mFullyLoaded = FALSE;
	mDesiredDiscardLevel = MAX_DISCARD_LEVEL + 1;
	mMinDesiredDiscardLevel = MAX_DISCARD_LEVEL + 1;
	mCalculatedDiscardLevel = -1.f;

	mDecodingAux = FALSE;

	mKnownDrawWidth = 0;
	mKnownDrawHeight = 0;

	if (firstinit)
	{
		mDecodePriority = 0.f;
		mInImageList = 0;
	}
	mIsMediaTexture = FALSE;

	mBoostLevel = LLViewerImage::BOOST_NONE;
	
	// Only set mIsMissingAsset true when we know for certain that the database
	// does not contain this image.
	mIsMissingAsset = FALSE;

	mNeedsCreateTexture = FALSE;
	
	mIsRawImageValid = FALSE;
	mRawDiscardLevel = INVALID_DISCARD_LEVEL;
	mMinDiscardLevel = 0;

	mTargetHost = LLHost::invalid;

	mHasFetcher = FALSE;
	mIsFetching = FALSE;
	mFetchState = 0;
	mFetchPriority = 0;
	mDownloadProgress = 0.f;
	mFetchDeltaTime = 999999.f;
	mDecodeFrame = 0;
	mVisibleFrame = 0;
}

// virtual
void LLViewerImage::dump()
{
	LLImageGL::dump();

	llinfos << "LLViewerImage"
			<< " mID " << mID
			<< " mIsMissingAsset " << (S32)mIsMissingAsset
			<< " mFullWidth " << mFullWidth
			<< " mFullHeight " << mFullHeight
			<< " mOrigWidth" << mOrigWidth
			<< " mOrigHeight" << mOrigHeight
			<< llendl;
}

///////////////////////////////////////////////////////////////////////////////

LLViewerImage::~LLViewerImage()
{
	if (mHasFetcher)
	{
		LLAppViewer::getTextureFetch()->deleteRequest(getID(), true);
	}
	// Explicitly call LLViewerImage::cleanup since we're in a destructor and cleanup is virtual
	LLViewerImage::cleanup();
	sImageCount--;
}


///////////////////////////////////////////////////////////////////////////////

void LLViewerImage::cleanup()
{
	for(callback_list_t::iterator iter = mLoadedCallbackList.begin();
		iter != mLoadedCallbackList.end(); )
	{
		LLLoadedCallbackEntry *entryp = *iter++;
		// We never finished loading the image.  Indicate failure.
		// Note: this allows mLoadedCallbackUserData to be cleaned up.
		entryp->mCallback( FALSE, this, NULL, NULL, 0, TRUE, entryp->mUserData );
		delete entryp;
	}
	mLoadedCallbackList.clear();
	mNeedsAux = FALSE;
	
	// Clean up image data
	destroyRawImage();
	
	// LLImageGL::cleanup will get called more than once when this is used in the destructor.
	LLImageGL::cleanup();
}

void LLViewerImage::reinit(BOOL usemipmaps /* = TRUE */)
{
	LLViewerImage::cleanup();
	LLImageGL::init(usemipmaps);
	init(false);
	setSize(0,0,0);
}

///////////////////////////////////////////////////////////////////////////////

// ONLY called from LLViewerImageList
BOOL LLViewerImage::createTexture(S32 usename/*= 0*/)
{
	if (!mNeedsCreateTexture)
	{
		destroyRawImage();
		return FALSE;
	}
	mNeedsCreateTexture	= FALSE;
	if (mRawImage.isNull())
	{
		llerrs << "LLViewerImage trying to create texture with no Raw Image" << llendl;
	}
// 	llinfos << llformat("IMAGE Creating (%d) [%d x %d] Bytes: %d ",
// 						mRawDiscardLevel, 
// 						mRawImage->getWidth(), mRawImage->getHeight(),mRawImage->getDataSize())
// 			<< mID.getString() << llendl;
	BOOL res = TRUE;
	if (!gNoRender)
	{
		// store original size only for locally-sourced images
		if (!mLocalFileName.empty())
		{
			mOrigWidth = mRawImage->getWidth();
			mOrigHeight = mRawImage->getHeight();

			// leave black border, do not scale image content
			mRawImage->expandToPowerOfTwo(MAX_IMAGE_SIZE, FALSE);
		}
		else
		{
			mOrigWidth = mFullWidth;
			mOrigHeight = mFullHeight;
		}


		if (LLImageGL::checkSize(mRawImage->getWidth(), mRawImage->getHeight()))
		{
			res = LLImageGL::createGLTexture(mRawDiscardLevel, mRawImage, usename);
		}
		else
		{
			// A non power-of-two image was uploaded (through a non standard client)
			// We treat these images as missing assets which causes them to
			// be renderd as 'missing image' and to stop requesting data
			setIsMissingAsset();
			destroyRawImage();
			return FALSE;
		}
	}

	//
	// Iterate through the list of image loading callbacks to see
	// what sort of data they need.
	//
	// *TODO: Fix image callback code
	BOOL imageraw_callbacks = FALSE;
	for(callback_list_t::iterator iter = mLoadedCallbackList.begin();
		iter != mLoadedCallbackList.end(); )
	{
		LLLoadedCallbackEntry *entryp = *iter++;
		if (entryp->mNeedsImageRaw)
		{
			imageraw_callbacks = TRUE;
			break;
		}
	}

	if (!imageraw_callbacks)
	{
		mNeedsAux = FALSE;
		destroyRawImage();
	}
	return res;
}

//============================================================================

void LLViewerImage::addTextureStats(F32 virtual_size) const // = 1.0
{
	if (virtual_size > mMaxVirtualSize)
	{
		mMaxVirtualSize = virtual_size;
	}
}

void LLViewerImage::resetTextureStats(BOOL zero)
{
	if (zero)
	{
		mMaxVirtualSize = 0.0f;
	}
	else
	{
		mMaxVirtualSize -= mMaxVirtualSize * .10f; // decay by 5%/update
	}
}

// This is gauranteed to get called periodically for every texture
void LLViewerImage::processTextureStats()
{
	// Generate the request priority and render priority
	if (mDontDiscard || !getUseMipMaps())
	{
		mDesiredDiscardLevel = 0;
		if (mFullWidth > MAX_IMAGE_SIZE_DEFAULT || mFullHeight > MAX_IMAGE_SIZE_DEFAULT)
			mDesiredDiscardLevel = 1; // MAX_IMAGE_SIZE_DEFAULT = 1024 and max size ever is 2048
	}
	else if (mBoostLevel < LLViewerImage::BOOST_HIGH && mMaxVirtualSize <= 10.f)
	{
		// If the image has not been significantly visible in a while, we don't want it
		mDesiredDiscardLevel = llmin(mMinDesiredDiscardLevel, (S8)(MAX_DISCARD_LEVEL + 1));
	}
	else if ((!mFullWidth && !getCurrentWidth())  || (!mFullHeight && !getCurrentHeight()))
	{
		mDesiredDiscardLevel = 	mMaxDiscardLevel;
	}
	else
	{
		//static const F64 log_2 = log(2.0);
		static const F64 log_4 = log(4.0);

		S32 fullwidth = llmin(mFullWidth,(S32)MAX_IMAGE_SIZE_DEFAULT);
		S32 fullheight = llmin(mFullHeight,(S32)MAX_IMAGE_SIZE_DEFAULT);
		mTexelsPerImage = (F32)fullwidth * fullheight;

		F32 discard_level = 0.f;

		// If we know the output width and height, we can force the discard
		// level to the correct value, and thus not decode more texture
		// data than we need to.
		if (mBoostLevel == LLViewerImage::BOOST_UI ||
			mBoostLevel == LLViewerImage::BOOST_PREVIEW ||
			mBoostLevel == LLViewerImage::BOOST_AVATAR_SELF)	// JAMESDEBUG what about AVATAR_BAKED_SELF?
		{
			discard_level = 0; // full res
		}
		else if (mKnownDrawWidth && mKnownDrawHeight)
		{
			S32 draw_texels = mKnownDrawWidth * mKnownDrawHeight;

			// Use log_4 because we're in square-pixel space, so an image
			// with twice the width and twice the height will have mTexelsPerImage
			// 4 * draw_size
			discard_level = (F32)(log(mTexelsPerImage/draw_texels) / log_4);
		}
		else
		{
			if ((mCalculatedDiscardLevel >= 0.f) &&
				(llabs(mMaxVirtualSize - mDiscardVirtualSize) < mMaxVirtualSize*.20f))
			{
				// < 20% change in virtual size = no change in desired discard
				discard_level = mCalculatedDiscardLevel; 
			}
			else
			{
				// Calculate the required scale factor of the image using pixels per texel
				discard_level = (F32)(log(mTexelsPerImage/mMaxVirtualSize) / log_4);
				mDiscardVirtualSize = mMaxVirtualSize;
				mCalculatedDiscardLevel = discard_level;
			}
		}
		if (mBoostLevel < LLViewerImage::BOOST_HIGH)
		{
			static const F32 discard_bias = -.5f; // Must be < 1 or highest discard will never load!
			discard_level += discard_bias;
			discard_level += sDesiredDiscardBias;
			discard_level *= sDesiredDiscardScale; // scale
		}
		discard_level = floorf(discard_level);
// 		discard_level -= (gImageList.mVideoMemorySetting>>1); // more video ram = higher detail

		F32 min_discard = 0.f;
		if (mFullWidth > MAX_IMAGE_SIZE_DEFAULT || mFullHeight > MAX_IMAGE_SIZE_DEFAULT)
			min_discard = 1.f; // MAX_IMAGE_SIZE_DEFAULT = 1024 and max size ever is 2048

		discard_level = llclamp(discard_level, min_discard, (F32)MAX_DISCARD_LEVEL);
		
		// Can't go higher than the max discard level
		mDesiredDiscardLevel = llmin((S32)mMaxDiscardLevel+1, (S32)discard_level);
		// Clamp to min desired discard
		mDesiredDiscardLevel = llmin(mMinDesiredDiscardLevel, mDesiredDiscardLevel);

		//
		// At this point we've calculated the quality level that we want,
		// if possible.  Now we check to see if we have it, and take the
		// proper action if we don't.
		//

		BOOL increase_discard = FALSE;
		S32 current_discard = getDiscardLevel();
		if ((sDesiredDiscardBias > 0.0f) &&
			(current_discard >= 0 && mDesiredDiscardLevel >= current_discard))
		{
			if ( (sBoundTextureMemory >> 20) > sMaxBoundTextureMem*texmem_middle_bound_scale)
			{
				// Limit the amount of GL memory bound each frame
				if (mDesiredDiscardLevel > current_discard)
				{
					increase_discard = TRUE;
				}
			}
			if ( (sTotalTextureMemory >> 20) > sMaxTotalTextureMem*texmem_middle_bound_scale)
			{
				// Only allow GL to have 2x the video card memory
				if (!getBoundRecently())
				{
					increase_discard = TRUE;
				}
			}
			if (increase_discard)
			{
				// 			llinfos << "DISCARDED: " << mID << " Discard: " << current_discard << llendl;
				sBoundTextureMemory -= mTextureMemory;
				sTotalTextureMemory -= mTextureMemory;
				// Increase the discard level (reduce the texture res)
				S32 new_discard = current_discard+1;
				setDiscardLevel(new_discard);
				sBoundTextureMemory += mTextureMemory;
				sTotalTextureMemory += mTextureMemory;
			}
		}
	}
}

//============================================================================

F32 LLViewerImage::calcDecodePriority()
{
#ifndef LL_RELEASE_FOR_DOWNLOAD
	if (mID == LLAppViewer::getTextureFetch()->mDebugID)
	{
		LLAppViewer::getTextureFetch()->mDebugCount++; // for setting breakpoints
	}
#endif
	
	if (mNeedsCreateTexture)
	{
		return mDecodePriority; // no change while waiting to create
	}

	F32 priority;
	S32 cur_discard = getDiscardLevel();
	bool have_all_data = (cur_discard >= 0 && (cur_discard <= mDesiredDiscardLevel));
	F32 pixel_priority = fsqrtf(mMaxVirtualSize);
	const S32 MIN_NOT_VISIBLE_FRAMES = 30; // NOTE: this function is not called every frame
	mDecodeFrame++;
	if (pixel_priority > 0.f)
	{
		mVisibleFrame = mDecodeFrame;
	}
	
	if (mIsMissingAsset)
	{
		priority = 0.0f;
	}
	else if (mDesiredDiscardLevel > mMaxDiscardLevel)
	{
		// Don't decode anything we don't need
		priority = -1.0f;
	}
	else if (mBoostLevel == LLViewerImage::BOOST_UI && !have_all_data)
	{
		priority = 1.f;
	}
	else if (pixel_priority <= 0.f && !have_all_data)
	{
		// Not on screen but we might want some data
		if (mBoostLevel > BOOST_HIGH)
		{
			// Always want high boosted images
			priority = 1.f;
		}
		else if (mVisibleFrame == 0 || (mDecodeFrame - mVisibleFrame > MIN_NOT_VISIBLE_FRAMES))
		{
			// Don't decode anything that isn't visible unless it's important
			priority = -2.0f;
		}
		else
		{
			// Leave the priority as-is
			return mDecodePriority;
		}
	}
	else if (cur_discard < 0)
	{
		// We don't have any data yet, so we don't know the size of the image, treat as 32x32
//		priority = 900000.f;
		static const F64 log_2 = log(2.0);
		F32 desired = (F32)(log(32.0/pixel_priority) / log_2);
		S32 ddiscard = MAX_DISCARD_LEVEL - (S32)desired + 1;
		ddiscard = llclamp(ddiscard, 1, 9);
		priority = ddiscard*100000.f;
	}
	else if ((mMinDiscardLevel > 0) && (cur_discard <= mMinDiscardLevel))
	{
		// larger mips are corrupted
		priority = -3.0f;
	}
	else if (cur_discard <= mDesiredDiscardLevel)
	{
		priority = -4.0f;
	}
	else
	{
		// priority range = 100000-400000
		S32 ddiscard = cur_discard - mDesiredDiscardLevel;
		if (getDontDiscard())
		{
			ddiscard+=2;
		}
		else if (!getBoundRecently() && mBoostLevel == 0)
		{
			ddiscard-=2;
		}
		ddiscard = llclamp(ddiscard, 0, 4);
		priority = ddiscard*100000.f;
	}
	if (priority > 0.0f)
	{
		pixel_priority = llclamp(pixel_priority, 0.0f, priority-1.f); // priority range = 100000-900000
		if ( mBoostLevel > BOOST_HIGH)
		{
			priority = 1000000.f + pixel_priority + 1000.f * mBoostLevel;
		}
		else
		{
			priority +=      0.f + pixel_priority + 1000.f * mBoostLevel;
		}
	}
	return priority;
}

// A value >= max value calculated above for normalization
//static
F32 LLViewerImage::maxDecodePriority()
{
	return 2000000.f;
}

void LLViewerImage::setDecodePriority(F32 priority)
{
	llassert(!mInImageList);
	mDecodePriority = priority;
}

void LLViewerImage::setBoostLevel(S32 level)
{
	mBoostLevel = level;
	if (level >= LLViewerImage::BOOST_HIGH)
	{
		processTextureStats();
	}
}

//============================================================================

bool LLViewerImage::updateFetch()
{
	mFetchState = 0;
	mFetchPriority = 0;
	mFetchDeltaTime = 999999.f;
	mRequestDeltaTime = 999999.f;

#ifndef LL_RELEASE_FOR_DOWNLOAD
	if (mID == LLAppViewer::getTextureFetch()->mDebugID)
	{
		LLAppViewer::getTextureFetch()->mDebugCount++; // for setting breakpoints
	}
#endif
	
	if (mIsMediaTexture)
	{
		llassert_always(!mHasFetcher);
		return false; // skip
	}
	if (mNeedsCreateTexture)
	{
		// We may be fetching still (e.g. waiting on write)
		// but don't check until we've processed the raw data we have
		return false;
	}
	if (mFullyLoaded)
	{
		llassert_always(!mHasFetcher);
		return false;
	}
	if (mIsMissingAsset)
	{
		llassert_always(!mHasFetcher);
		return false; // skip
	}
	if (!mLoadedCallbackList.empty() && mRawImage.notNull())
	{
		return false; // process any raw image data in callbacks before replacing
	}
	
	S32 current_discard = getDiscardLevel();
	S32 desired_discard = getDesiredDiscardLevel();
	F32 decode_priority = getDecodePriority();
	decode_priority = llmax(decode_priority, 0.0f);
	
	if (mIsFetching)
	{
		// Sets mRawDiscardLevel, mRawImage, mAuxRawImage
		S32 fetch_discard = current_discard;
		if (mRawImage.notNull()) sRawCount--;
		if (mAuxRawImage.notNull()) sAuxCount--;
		bool finished = LLAppViewer::getTextureFetch()->getRequestFinished(getID(), fetch_discard, mRawImage, mAuxRawImage);
		if (mRawImage.notNull()) sRawCount++;
		if (mAuxRawImage.notNull()) sAuxCount++;
		if (finished)
		{
			mIsFetching = FALSE;
		}
		else
		{
			mFetchState = LLAppViewer::getTextureFetch()->getFetchState(mID, mDownloadProgress, mRequestedDownloadPriority,
																		mFetchPriority, mFetchDeltaTime, mRequestDeltaTime);
		}
		
		// We may have data ready regardless of whether or not we are finished (e.g. waiting on write)
		if (mRawImage.notNull())
		{
			mRawDiscardLevel = fetch_discard;
			if ((mRawImage->getDataSize() > 0 && mRawDiscardLevel >= 0) &&
				(current_discard < 0 || mRawDiscardLevel < current_discard))
			{
				if (getComponents() != mRawImage->getComponents())
				{
					// We've changed the number of components, so we need to move any
					// objects using this pool to a different pool.
					mComponents = mRawImage->getComponents();
					gImageList.dirtyImage(this);
				}			
				mIsRawImageValid = TRUE;
				gImageList.mCreateTextureList.insert(this);
				mNeedsCreateTexture = TRUE;
				mFullWidth = mRawImage->getWidth() << mRawDiscardLevel;
				mFullHeight = mRawImage->getHeight() << mRawDiscardLevel;
			}
			else
			{
				// Data is ready but we don't need it
				// (received it already while fetcher was writing to disk)
				destroyRawImage();
				return false; // done
			}
		}
		
		if (!mIsFetching)
		{
			if ((decode_priority > 0) && (mRawDiscardLevel < 0 || mRawDiscardLevel == INVALID_DISCARD_LEVEL))
			{
				// We finished but received no data
				if (current_discard < 0)
				{
					setIsMissingAsset();
					desired_discard = -1;
				}
				else
				{
					llwarns << mID << ": Setting min discard to " << current_discard << llendl;
					mMinDiscardLevel = current_discard;
					desired_discard = current_discard;
				}
				destroyRawImage();
			}
			else if (mRawImage.isNull())
			{
				// We have data, but our fetch failed to return raw data
				// *TODO: FIgure out why this is happening and fix it
				destroyRawImage();
			}
		}
		else
		{
			LLAppViewer::getTextureFetch()->updateRequestPriority(mID, decode_priority);
		}
	}

	bool make_request = true;
	
	if (decode_priority <= 0)
	{
		make_request = false;
	}
	else if (mNeedsCreateTexture || mIsMissingAsset)
	{
		make_request = false;
	}
	else if (current_discard >= 0 && current_discard <= mMinDiscardLevel)
	{
		make_request = false;
	}
	else
	{
		if (mIsFetching)
		{
			if (mRequestedDiscardLevel <= desired_discard)
			{
				make_request = false;
			}
		}
		else
		{
			if (current_discard >= 0 && current_discard <= desired_discard)
			{
				make_request = false;
			}
		}
	}
	
	if (make_request)
	{
		S32 w=0, h=0, c=0;
		if (current_discard >= 0)
		{
			w = getWidth(0);
			h = getHeight(0);
			c = getComponents();
		}
		if (!mDontDiscard)
		{
			if (mBoostLevel == 0)
			{
				desired_discard = llmax(desired_discard, current_discard-1);
			}
			else
			{
				desired_discard = llmax(desired_discard, current_discard-2);
			}
		}

		// bypass texturefetch directly by pulling from LLTextureCache
		bool fetch_request_created = false;
		if (mLocalFileName.empty())
		{
			fetch_request_created = LLAppViewer::getTextureFetch()->createRequest(getID(), getTargetHost(), decode_priority,
											 w, h, c, desired_discard,
											 needsAux());
		}
		else
		{
			fetch_request_created = LLAppViewer::getTextureFetch()->createRequest(mLocalFileName, getID(),getTargetHost(), decode_priority,
											 w, h, c, desired_discard,
											 needsAux());
		}

		if (fetch_request_created)
		{
			mHasFetcher = TRUE;
			mIsFetching = TRUE;
			mRequestedDiscardLevel = desired_discard;
			mFetchState = LLAppViewer::getTextureFetch()->getFetchState(mID, mDownloadProgress, mRequestedDownloadPriority,
													   mFetchPriority, mFetchDeltaTime, mRequestDeltaTime);
		}

		// if createRequest() failed, we're finishing up a request for this UUID,
		// wait for it to complete
	}
	else if (mHasFetcher && !mIsFetching)
	{
		// Only delete requests that haven't receeived any network data for a while
		const F32 FETCH_IDLE_TIME = 5.f;
		if (mLastPacketTimer.getElapsedTimeF32() > FETCH_IDLE_TIME)
		{
// 			llinfos << "Deleting request: " << getID() << " Discard: " << current_discard << " <= min:" << mMinDiscardLevel << " or priority == 0: " << decode_priority << llendl;
			LLAppViewer::getTextureFetch()->deleteRequest(getID(), true);
			mHasFetcher = FALSE;
		}
	}
	
	llassert_always(mRawImage.notNull() || (!mNeedsCreateTexture && !mIsRawImageValid));
	
	return mIsFetching ? true : false;
}

void LLViewerImage::setIsMissingAsset()
{
	llwarns << mLocalFileName << " " << mID << ": Marking image as missing" << llendl;
	if (mHasFetcher)
	{
		LLAppViewer::getTextureFetch()->deleteRequest(getID(), true);
		mHasFetcher = FALSE;
		mIsFetching = FALSE;
		mFetchState = 0;
		mFetchPriority = 0;
	}
	mIsMissingAsset = TRUE;
}

//============================================================================

void LLViewerImage::setLoadedCallback( loaded_callback_func loaded_callback,
									   S32 discard_level, BOOL keep_imageraw, BOOL needs_aux, void* userdata)
{
	//
	// Don't do ANYTHING here, just add it to the global callback list
	//
	if (mLoadedCallbackList.empty())
	{
		// Put in list to call this->doLoadedCallbacks() periodically
		gImageList.mCallbackList.insert(this);
	}
	LLLoadedCallbackEntry* entryp = new LLLoadedCallbackEntry(loaded_callback, discard_level, keep_imageraw, userdata);
	mLoadedCallbackList.push_back(entryp);
	mNeedsAux |= needs_aux;
	if (mNeedsAux && mAuxRawImage.isNull() && getDiscardLevel() >= 0)
	{
		// We need aux data, but we've already loaded the image, and it didn't have any
		llwarns << "No aux data available for callback for image:" << getID() << llendl;
	}
}

bool LLViewerImage::doLoadedCallbacks()
{
	if (mNeedsCreateTexture)
	{
		return false;
	}

	bool res = false;
	
	if (isMissingAsset())
	{
		for(callback_list_t::iterator iter = mLoadedCallbackList.begin();
			iter != mLoadedCallbackList.end(); )
		{
			LLLoadedCallbackEntry *entryp = *iter++;
			// We never finished loading the image.  Indicate failure.
			// Note: this allows mLoadedCallbackUserData to be cleaned up.
			entryp->mCallback(FALSE, this, NULL, NULL, 0, TRUE, entryp->mUserData);
			delete entryp;
		}
		mLoadedCallbackList.clear();

		// Remove ourself from the global list of textures with callbacks
		gImageList.mCallbackList.erase(this);
	}

	S32 gl_discard = getDiscardLevel();

	// If we don't have a legit GL image, set it to be lower than the worst discard level
	if (gl_discard == -1)
	{
		gl_discard = MAX_DISCARD_LEVEL + 1;
	}

	//
	// Determine the quality levels of textures that we can provide to callbacks
	// and whether we need to do decompression/readback to get it
	//
	S32 current_raw_discard = MAX_DISCARD_LEVEL + 1; // We can always do a readback to get a raw discard
	S32 best_raw_discard = gl_discard;	// Current GL quality level
	S32 current_aux_discard = MAX_DISCARD_LEVEL + 1;
	S32 best_aux_discard = MAX_DISCARD_LEVEL + 1;

	if (mIsRawImageValid)
	{
		// If we have an existing raw image, we have a baseline for the raw and auxiliary quality levels.
		best_raw_discard = llmin(best_raw_discard, mRawDiscardLevel);
		best_aux_discard = llmin(best_aux_discard, mRawDiscardLevel); // We always decode the aux when we decode the base raw
		current_aux_discard = llmin(current_aux_discard, best_aux_discard);
	}
	else
	{
		// We have no data at all, we need to get it
		// Do this by forcing the best aux discard to be 0.
		best_aux_discard = 0;
	}


	//
	// See if any of the callbacks would actually run using the data that we can provide,
	// and also determine if we need to perform any readbacks or decodes.
	//
	bool run_gl_callbacks = false;
	bool run_raw_callbacks = false;
	bool need_readback = false;

	for(callback_list_t::iterator iter = mLoadedCallbackList.begin();
		iter != mLoadedCallbackList.end(); )
	{
		LLLoadedCallbackEntry *entryp = *iter++;
		if (entryp->mNeedsImageRaw)
		{
			if (mNeedsAux)
			{
				//
				// Need raw and auxiliary channels
				//
				if (entryp->mLastUsedDiscard > current_aux_discard)
				{
					// We have useful data, run the callbacks
					run_raw_callbacks = true;
				}
			}
			else
			{
				if (entryp->mLastUsedDiscard > current_raw_discard)
				{
					// We have useful data, just run the callbacks
					run_raw_callbacks = true;
				}
				else if (entryp->mLastUsedDiscard > best_raw_discard)
				{
					// We can readback data, and then run the callbacks
					need_readback = true;
					run_raw_callbacks = true;
				}
			}
		}
		else
		{
			// Needs just GL
			if (entryp->mLastUsedDiscard > gl_discard)
			{
				// We have enough data, run this callback requiring GL data
				run_gl_callbacks = true;
			}
		}
	}

	//
	// Do a readback if required, OR start off a texture decode
	//
	if (need_readback && (mMaxDiscardLevel > gl_discard))
	{
		// Do a readback to get the GL data into the raw image
		// We have GL data.

		destroyRawImage();
		readBackRawImage(gl_discard);
		llassert_always(mRawImage.notNull());
		llassert_always(!mNeedsAux || mAuxRawImage.notNull());
	}

	//
	// Run raw/auxiliary data callbacks
	//
	if (run_raw_callbacks && mIsRawImageValid && (mRawDiscardLevel <= mMaxDiscardLevel))
	{
		// Do callbacks which require raw image data.
		//llinfos << "doLoadedCallbacks raw for " << getID() << llendl;

		// Call each party interested in the raw data.
		for(callback_list_t::iterator iter = mLoadedCallbackList.begin();
			iter != mLoadedCallbackList.end(); )
		{
			callback_list_t::iterator curiter = iter++;
			LLLoadedCallbackEntry *entryp = *curiter;
			if (entryp->mNeedsImageRaw && (entryp->mLastUsedDiscard > mRawDiscardLevel))
			{
				// If we've loaded all the data there is to load or we've loaded enough
				// to satisfy the interested party, then this is the last time that
				// we're going to call them.

				llassert_always(mRawImage.notNull());
				if(mNeedsAux && mAuxRawImage.isNull())
				{
					llwarns << "Raw Image with no Aux Data for callback" << llendl;
				}
				BOOL final = mRawDiscardLevel <= entryp->mDesiredDiscard ? TRUE : FALSE;
				//llinfos << "Running callback for " << getID() << llendl;
				//llinfos << mRawImage->getWidth() << "x" << mRawImage->getHeight() << llendl;
				if (final)
				{
					//llinfos << "Final!" << llendl;
				}
				entryp->mLastUsedDiscard = mRawDiscardLevel;
				entryp->mCallback(TRUE, this, mRawImage, mAuxRawImage, mRawDiscardLevel, final, entryp->mUserData);
				if (final)
				{
					iter = mLoadedCallbackList.erase(curiter);
					delete entryp;
				}
				res = true;
			}
		}
	}

	//
	// Run GL callbacks
	//
	if (run_gl_callbacks && (gl_discard <= mMaxDiscardLevel))
	{
		//llinfos << "doLoadedCallbacks GL for " << getID() << llendl;

		// Call the callbacks interested in GL data.
		for(callback_list_t::iterator iter = mLoadedCallbackList.begin();
			iter != mLoadedCallbackList.end(); )
		{
			callback_list_t::iterator curiter = iter++;
			LLLoadedCallbackEntry *entryp = *curiter;
			if (!entryp->mNeedsImageRaw && (entryp->mLastUsedDiscard > gl_discard))
			{
				BOOL final = gl_discard <= entryp->mDesiredDiscard ? TRUE : FALSE;
				entryp->mLastUsedDiscard = gl_discard;
				entryp->mCallback(TRUE, this, NULL, NULL, gl_discard, final, entryp->mUserData);
				if (final)
				{
					iter = mLoadedCallbackList.erase(curiter);
					delete entryp;
				}
				res = true;
			}
		}
	}

	//
	// If we have no callbacks, take us off of the image callback list.
	//
	if (mLoadedCallbackList.empty())
	{
		gImageList.mCallbackList.erase(this);
	}

	// Done with any raw image data at this point (will be re-created if we still have callbacks)
	destroyRawImage();
	
	return res;
}

//============================================================================

// Call with 0,0 to turn this feature off.
void LLViewerImage::setKnownDrawSize(S32 width, S32 height)
{
	mKnownDrawWidth = width;
	mKnownDrawHeight = height;
	addTextureStats((F32)(width * height));
}

// virtual
bool LLViewerImage::bindError(S32 stage) const
{
	if (stage < 0) return false;
	
	if (gNoRender)
	{
		return false;
	}

	bool res = true;
	
	// On failure to bind, what should we set the currently bound texture to?
	if (mIsMissingAsset && !sMissingAssetImagep.isNull() && (this != (LLImageGL *)sMissingAssetImagep))
	{
		res = gGL.getTexUnit(stage)->bind(sMissingAssetImagep.get());
	}
	if (!res && !sDefaultImagep.isNull() && (this != (LLImageGL *)sDefaultImagep))
	{
		// use default if we've got it
		res = gGL.getTexUnit(stage)->bind(sDefaultImagep.get());
	}
	if (!res && !sNullImagep.isNull() && (this != (LLImageGL *)sNullImagep))
	{
		res = gGL.getTexUnit(stage)->bind(sNullImagep.get());
	}
	if (!res)
	{
		llwarns << "LLViewerImage::bindError failed." << llendl;
	}
	stop_glerror();
	return res;
}

bool LLViewerImage::bindDefaultImage(S32 stage) const
{
	if (stage < 0) return false;

	bool res = true;
	if (!sDefaultImagep.isNull() && (this != (LLImageGL *)sDefaultImagep))
	{
		// use default if we've got it
		res = gGL.getTexUnit(stage)->bind(sDefaultImagep.get());
	}
	if (!res && !sNullImagep.isNull() && (this != (LLImageGL *)sNullImagep))
	{
		res = gGL.getTexUnit(stage)->bind(sNullImagep.get());
	}
	if (!res)
	{
		llwarns << "LLViewerImage::bindError failed." << llendl;
	}
	stop_glerror();
	return res;
}

// Was in LLImageGL
LLImageRaw* LLViewerImage::readBackRawImage(S8 discard_level)
{
	llassert_always(discard_level >= 0);
	llassert_always(mComponents > 0);
	if (mRawImage.notNull())
	{
		llerrs << "called with existing mRawImage" << llendl;
		mRawImage = NULL;
	}
	mRawImage = new LLImageRaw(getWidth(discard_level), getHeight(discard_level), mComponents);
	sRawCount++;
	mRawDiscardLevel = discard_level;
	readBackRaw(mRawDiscardLevel, mRawImage, false);
	mIsRawImageValid = TRUE;
	
	return mRawImage;
}

void LLViewerImage::destroyRawImage()
{
	if (mRawImage.notNull()) sRawCount--;
	if (mAuxRawImage.notNull()) sAuxCount--;
	mRawImage = NULL;
	mAuxRawImage = NULL;
	mIsRawImageValid = FALSE;
	mRawDiscardLevel = INVALID_DISCARD_LEVEL;
}
