/** 
 * @file llviewerimage.cpp
 * @brief Object which handles a received image (and associated texture(s))
 *
 * Copyright (c) 2000-2007, Linden Research, Inc.
 * 
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

#include "llviewerprecompiledheaders.h"

#include "llviewerimage.h"

// Library includes
#include "imageids.h"
#include "llmath.h"
#include "llerror.h"
#include "llgl.h"
#include "llhost.h"
#include "llimage.h"
#include "llimagebmp.h"
#include "llimagej2c.h"
#include "llimagetga.h"
#include "llstl.h"
#include "lltexturetable.h"
#include "llvfile.h"
#include "llvfs.h"
#include "message.h"
#include "lltimer.h"
//#include "vmath.h"			// For frand

// viewer includes
#include "llviewerimagelist.h"
#include "llviewercontrol.h"
#include "viewer.h"
#include "llglheaders.h"
#include "pipeline.h"
#include "lldrawpool.h"

const S32 IMAGE_HEADER_SIZE = 27;
const S32 PACKET_HEADER_SIZE = 4;

///////////////////////////////////////////////////////////////////////////////

class LLViewerImagePacket
{
public:
	LLViewerImagePacket(U8 *data, U16 data_size, U16 packet_num, BOOL wrote_to_disk)
	{
		mData = data;
		mDataSize = data_size;
		mPacketNum = packet_num;
		mWroteToDisk = wrote_to_disk;
	}

	~LLViewerImagePacket()
	{
		delete[] mData;
	}

public:
	U8	*mData;
	U16	mDataSize;
	U16	mPacketNum;
	BOOL mWroteToDisk;
};


///////////////////////////////////////////////////////////////////////////////

// statics
LLPointer<LLViewerImage> LLViewerImage::sMissingAssetImagep = NULL;
LLPointer<LLViewerImage> LLViewerImage::sWhiteImagep = NULL;
LLPointer<LLImageGL> LLViewerImage::sDefaultImagep = NULL;
LLPointer<LLViewerImage> LLViewerImage::sSmokeImagep = NULL;
LLPointer<LLImageGL> LLViewerImage::sNullImagep = NULL;

S32 LLViewerImage::sImageCount = 0;
LLTimer LLViewerImage::sEvaluationTimer;
F32 LLViewerImage::sDesiredDiscardBias = 0.f;
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
	sBoundTextureMemory = LLImageGL::sBoundTextureMemory;
	sTotalTextureMemory = LLImageGL::sGlobalTextureMemory;
	sMaxBoundTextureMem = gImageList.getMaxResidentTexMem();
	
	sMaxTotalTextureMem = sMaxBoundTextureMem * 2;
	if (sMaxBoundTextureMem > 64000000)
	{
		sMaxTotalTextureMem -= sMaxBoundTextureMem/4;
	}
	
	if (sMaxTotalTextureMem > (S32)gSysMemory.getPhysicalMemory() - min_non_tex_system_mem)
	{
		sMaxTotalTextureMem = (S32)gSysMemory.getPhysicalMemory() - min_non_tex_system_mem;
	}
	
	if (sBoundTextureMemory >= sMaxBoundTextureMem ||
		sTotalTextureMemory >= sMaxTotalTextureMem)
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
			 sBoundTextureMemory < sMaxBoundTextureMem*texmem_lower_bound_scale &&
			 sTotalTextureMemory < sMaxTotalTextureMem*texmem_lower_bound_scale)
	{
		// If we are using less texture memory than we should,
		// scale down the desired discard level
		if (sEvaluationTimer.getElapsedTimeF32() > discard_delta_time)
		{
			sDesiredDiscardBias -= discard_bias_delta;
			sEvaluationTimer.reset();
		}
	}
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

LLViewerImage::LLViewerImage(const U32 width, const U32 height, const U8 components, BOOL usemipmaps)
	: LLImageGL(width, height, components, usemipmaps)
{
	init(true);
	mNeedsAux = FALSE;
	// Create an empty image of the specified size and width
	mID.generate();
	mFullyLoaded = TRUE;
	sImageCount++;
}

LLViewerImage::LLViewerImage(const LLImageRaw* raw, BOOL usemipmaps)
	: LLImageGL(raw, usemipmaps)
{
	init(true);
	mNeedsAux = FALSE;
	// Create an empty image of the specified size and width
	mID.generate();
	mFullyLoaded = TRUE;
	sImageCount++;
}

void LLViewerImage::init(bool firstinit)
{
	mDataCodec = 0;
	mFullWidth = 0;
	mFullHeight = 0;
	mFormattedImagep = NULL;
	mNeedsAux = FALSE;
	mRequested = FALSE;
	mNeedsDecode = FALSE;
	mTexelsPerImage = 64.f*64.f;
	mMaxVirtualSize = 0.f;
	mMaxCosAngle = -1.f;
	mRequestedDiscardLevel = -1;
	mRequestedDownloadPriority = 0.f;
	mPackets = 0;
	mGotFirstPacket = FALSE;
	mPacketsReceived = 0;
	mFullyLoaded = FALSE;
	mDesiredDiscardLevel = MAX_DISCARD_LEVEL + 1;
	mMinDesiredDiscardLevel = MAX_DISCARD_LEVEL + 1;
	mStreamFile = NULL;
	mCachedData = NULL;
	mCachedSize = 0;
	mFormattedFlushed = FALSE;
	mTotalBytes = 0;

	resetPacketData();

	mLastPacketProcessed = -1;
	mLastBytesProcessed = 0;
	mLastPacket = -1;
	mDecodingAux = FALSE;

	mKnownDrawWidth = 0;
	mKnownDrawHeight = 0;

	if (firstinit)
	{
		mDecodePriority = 0.f;
		mInImageList = 0;
		mInStaticVFS = FALSE;
	}
	mIsMediaTexture = FALSE;

	mBoostLevel = LLViewerImage::BOOST_NONE;
	
	// Only set mIsMissingAsset true when we know for certain that the database
	// does not contain this image.
	mIsMissingAsset = FALSE;

	mNeedsCreateTexture = FALSE;
	
	mIsRawImageValid = FALSE;
	mRawDiscardLevel = INVALID_DISCARD_LEVEL;
	mRawImage = NULL;

	mTargetHost = LLHost::invalid;
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
			<< llendl;
}

///////////////////////////////////////////////////////////////////////////////

LLViewerImage::~LLViewerImage()
{
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

	// Clean up any remaining packet data.
	std::for_each(mReceivedPacketMap.begin(), mReceivedPacketMap.end(), DeletePairedPointer());
	mReceivedPacketMap.clear();

	// Clean up the streaming file
	if (mStreamFile && !mStreamFile->isReadComplete())
	{
// 		llwarns << "Destroying LLViewerImage stream file while still reading data!" << llendl;
	}
	delete mStreamFile;
	mStreamFile = NULL;

	// Clean up image data
	setFormattedImage(NULL);
	mRawImage = NULL;
	mIsRawImageValid = FALSE;
	mAuxRawImage = NULL;
	
	delete[] mCachedData;
	mCachedData = NULL;

	// LLImageGL::cleanup will get called more than once when this is used in the destructor.
	LLImageGL::cleanup();
}

void LLViewerImage::reinit(BOOL usemipmaps /* = TRUE */)
{
	LLViewerImage::cleanup();
	LLImageGL::init(usemipmaps);
	init(false);
	setSize(0,0,0);
	if (mInStaticVFS)
	{
		mFormattedFlushed = TRUE;
	}
}

///////////////////////////////////////////////////////////////////////////////

void LLViewerImage::setFormattedImage(LLImageFormatted* imagep)
{
	mFormattedImagep = NULL; // deletes image
	mFormattedImagep = imagep;
	if (mFormattedImagep.notNull())
	{
		mFormattedImagep->mMemType = LLMemType::MTYPE_APPFMTIMAGE;
		mFormattedFlushed = FALSE;
	}
	else
	{
		setNeedsDecode(FALSE);
	}
}

BOOL LLViewerImage::loadLocalImage(const LLUUID &image_id)
{
	LLMemType mt1(LLMemType::MTYPE_APPFMTIMAGE);
	
	// first look for this image in the static VFS
	LLAssetType::EType asset_type = LLAssetType::AT_NONE;
	// Try TGA first
	if (gStaticVFS->getExists(image_id, LLAssetType::AT_TEXTURE_TGA))
	{
		asset_type = LLAssetType::AT_TEXTURE_TGA;
		//RN: force disable discards for TGA files because they can't decode at different quality levels
		dontDiscard();
		mDataCodec = IMG_CODEC_TGA;
	}
	else if (gStaticVFS->getExists(image_id, LLAssetType::AT_TEXTURE))
	{
		// then try for a J2C version
		asset_type = LLAssetType::AT_TEXTURE;
		mDataCodec = IMG_CODEC_J2C;
		LLImageJ2C* imagej2c = new LLImageJ2C();
		setFormattedImage(imagej2c);
	}

	if (asset_type != LLAssetType::AT_NONE)
	{
		S32 size = gStaticVFS->getSize(image_id, asset_type);
		U8* buffer = new U8[size];
		BOOL success = LLVFSThread::sLocal->readImmediate(gStaticVFS, image_id, asset_type, buffer, 0, size);

		if (!success)
		{
			llwarns << "loadLocalImage() - vfs read failed" << llendl;
			return FALSE;
		}

		mInStaticVFS = TRUE;
		mFullyLoaded = TRUE;
		setNeedsDecode(TRUE); // Loading a local image
		mID = image_id;
		setDecodeData(buffer, size);
		mTotalBytes = size;
		mLastBytesProcessed = size;
		return TRUE;
	}

	return FALSE;
}

BOOL LLViewerImage::startVFSLoad()
{
	// We're no longer considered "flushed" no matter what happens after here.
	mFormattedFlushed = FALSE;
	if (!mStreamFile && mFormattedImagep.isNull())
	{
		// Start load from VFS if it's there
		if (gVFS->getExists(mID, LLAssetType::AT_TEXTURE))
		{
// 			llinfos << "Reading image from disk " << getID() << llendl;
			
			//llinfos << mID << ": starting VFS load" << llendl;
			mStreamFile = new LLVFile(gVFS, mID, LLAssetType::AT_TEXTURE, LLVFile::READ_WRITE);
			mCachedSize = 0;
			gImageList.mLoadingStreamList.push_back(this);
		}
		else
		{
			return loadLocalImage(mID);
		}
	}
	return TRUE;
}

void LLViewerImage::startImageDecode()
{
	// We need to load and/or decode the image
	if (mFormattedImagep.isNull())
	{
		startVFSLoad(); // Start the VFS loading
	}
	else
	{
		setNeedsDecode(TRUE); // Force a new decode of this texture
	}
}


BOOL LLViewerImage::createTexture(S32 usename/*= 0*/)
{
	if (mFormattedImagep.notNull() && mFormattedImagep->isDecoding())
	{
		llerrs << "Trying to create texture on an image that is currently being decoded: " << mID << llendl;
	}
	mNeedsCreateTexture	= FALSE;
	if (mRawImage.isNull())
	{
		llerrs << "LLViewerImage trying to create texture with no Raw Image" << llendl;
	}
// 	llinfos << llformat("IMAGE Creating (%d,%d) [%d x %d] Bytes: %d ",
// 						mRawDiscardLevel, mFormattedImagep ? mFormattedImagep->getDiscardLevel() : -1,
// 						mRawImage->getWidth(), mRawImage->getHeight(),mRawImage->getDataSize())
// 			<< mID.getString() << llendl;
	BOOL res = TRUE;
	if (!gNoRender)
	{
		if (LLImageGL::checkSize(mRawImage->getWidth(), mRawImage->getHeight()))
		{
			res = LLImageGL::createGLTexture(mRawDiscardLevel, mRawImage, usename);
		}
		else
		{
			// A non power-of-two image was uploaded (through a non standard client)
			// We treat these images as missing assets which causes them to
			// be renderd as 'missing image' and to stop requesting data
			setIsMissingAsset(TRUE);
			destroyRawImage();
			return FALSE;
		}
	}

	//
	// Iterate through the list of image loading callbacks to see
	// what sort of data they need.
	//
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
		destroyRawImage();
	}
	return res;
}

BOOL LLViewerImage::destroyTexture()
{
	LLImageGL::destroyGLTexture();
	return TRUE;
}

void LLViewerImage::resetPacketData()
{
	//llinfos << "resetting packet data for " << getID() << llendl;
	mPackets = 0;
	mLastPacket = -1;
	mPacketsReceived = 0;
	mGotFirstPacket = FALSE;
	mRequested = FALSE;

	std::for_each(mReceivedPacketMap.begin(), mReceivedPacketMap.end(), DeletePairedPointer());
	mReceivedPacketMap.clear();
	mLastPacketProcessed = -1;
}

////////////////////////////////////////////////////////////////////////////////
// This sets up a new formatted image at the requested size, and sets the decode flag
void LLViewerImage::setDecodeData(U8 * data, U32 size)
{
	if (size == mTotalBytes)
	{
		mFullyLoaded = TRUE;
	}
	if (mDataCodec == IMG_CODEC_J2C) 
	{
		//Codec 2 = compressed with JPEG2000 (Wavelet)
		// Create formatted image first, then use it to generate the
		// raw image.
		if (mFormattedImagep.isNull())
		{
			LLPointer<LLImageJ2C> j2cp = new LLImageJ2C();
			setFormattedImage(j2cp);
		}
		BOOL res = mFormattedImagep->setData(data, size);
		if (mFullyLoaded)
		{
			mFormattedImagep->setDiscardLevel(0); // Force full res if all data is loaded
		}
		if ((mFormattedImagep->getWidth() > MAX_IMAGE_SIZE_DEFAULT ||
			 mFormattedImagep->getHeight() > MAX_IMAGE_SIZE_DEFAULT) &&
			(mFormattedImagep->getDiscardLevel() == 0))
		{
			mFormattedImagep->setDiscardLevel(1); // Force x2048 images to x1024
		}
		
		if( res )
		{
			if (mFormattedImagep->getComponents() > 4)
			{
				mNeedsAux = TRUE;
			}
			else
			{
				mNeedsAux = FALSE;
			}
			mFullWidth = mFormattedImagep->getWidth();
			mFullHeight = mFormattedImagep->getHeight();
			if ((mFullWidth == 0) || (mFullHeight == 0))
			{
				llwarns << "Zero size width/height!" << llendl;
			}
			setNeedsDecode(TRUE); // Setting new formatted data
		}
		else
		{
			llwarns << "Unable to setData() for image " << mID << " Aborting." << llendl;
			abortDecode();
		}
	}
	else if (mDataCodec == IMG_CODEC_TGA)
	{
		//Codec 4 = compressed with TGA
		// Create formatted image first, then use it to generate the
		// raw image.
		if (mFormattedImagep.isNull())
		{
			LLPointer<LLImageTGA> tgap = new LLImageTGA();
			setFormattedImage(tgap);
		}
		BOOL res = mFormattedImagep->setData(data, size);
		
		if( res )
		{
			mFullWidth = mFormattedImagep->getWidth();
			mFullHeight = mFormattedImagep->getHeight();
			if ((mFullWidth == 0) || (mFullHeight == 0))
			{
				llwarns << "Zero size width/height!" << llendl;
			}
			setNeedsDecode(TRUE); // Setting new formatted data
		}
		else
		{
			llwarns << "Unable to setData() for image " << mID << " Aborting." << llendl;
			abortDecode();
		}
	}
	else 
	{
		llerrs << "Image " << mID << ": Unknown codec " << (int)mDataCodec << llendl;
		setNeedsDecode(FALSE); // Unknown codec
	}
}

void LLViewerImage::decodeImage(const F32 decode_time)
{
	if (!needsDecode())
	{
		return;
	}

	if (mFormattedImagep.isNull())
	{
		llerrs << "Decoding image without formatted data!" << llendl;
		return;
	}

	//
	// Only do a decode if we don't already have an image for this resolution. 
	//
	if (getTexName() != 0 && getDiscardLevel() <= mFormattedImagep->getDiscardLevel()
		&& !mNeedsAux)
	{
		// We already have an image this size or larger
		setNeedsDecode(FALSE);
		return;
	}

	// Partial Decode of J2C images:
	//  If this is the first time we are decoding an image,
	//  make sure we limit the amount of data we decode in order not to stall other decodes
	if (!mFormattedImagep->isDecoding() && mFormattedImagep->getCodec() == IMG_CODEC_J2C)
	{
		LLImageJ2C* j2cp = (LLImageJ2C*)((LLImageFormatted*)mFormattedImagep);
		const S32 INITIAL_DECODE_SIZE = 2048;
		if (!mDontDiscard &&
			getUseMipMaps() &&
			!mNeedsAux &&
			getDiscardLevel() < 0 &&
			mFormattedImagep->getDataSize() > INITIAL_DECODE_SIZE * 2)
		{
			j2cp->setMaxBytes(INITIAL_DECODE_SIZE);
		}
		else
		{
			j2cp->setMaxBytes(0); // In case we set it on a previous decode
		}
	}
	
	//
	// Decode Image
	//
	mLastDecodeTime.reset();
		
	if (mFormattedImagep->getCodec() == 0)
	{
		llerrs << "LLViewerImage::decodeImage: mFormattedImagep->getCodec() == 0" << llendl;
	}

	//
	// Decode first 4 channels
	//
	// Skip over this if we're already in the process of decoding the aux channel,
	// that means that we've alredy decoded the base channels of this texture.
	if (!mDecodingAux)
	{
		if (!mFormattedImagep->isDecoding())
		{
			mNeedsCreateTexture	= FALSE; // Raw is no longer valid
			destroyRawImage();
			createRawImage(mFormattedImagep->getDiscardLevel());
			//llinfos << "starting decode at " << (S32)mFormattedImagep->getDiscardLevel() << " for " << getID() << llendl;
		}
		else
		{
			llassert(mRawImage.notNull());
		}
		if (!mFormattedImagep->decode(mRawImage, decode_time, 0, 4))
		{
			if (!mFormattedImagep->isDecoding())
			{
				// bogus data, delete and try again
				llwarns << "Failed to decode " << mID << ":" << gTextureTable.getName(mID) << llendl;
				abortDecode();
				destroyRawImage();
				return;
			}
		}
// 		llinfos << llformat("IMAGE Decode (%d) ", mFormattedImagep->getDiscardLevel()) << mID << llendl;

		// Get the discard level of the decoded raw image,
		// which may not match the formatted image discard level if a partial decode was done
		mRawDiscardLevel = mFormattedImagep->getRawDiscardLevel();
		
		if (mFormattedImagep->isDecoding())
		{
			return; // Not done decoding.
		}
	}
				
	//
	// If we've finished with the main channels and need to decode the aux, do it now.
	// Aux buffers contain extra data (e.g. cloth maps)
	//
	if (mNeedsAux)
	{
		mDecodingAux = TRUE;

		// Create the target raw image for the aux channels
		if (mAuxRawImage.isNull())
		{
			S32 discard = mFormattedImagep->getDiscardLevel();
			mAuxRawImage = new LLImageRaw(getWidth(discard), getHeight(discard), 1);
			mAuxRawImage->mMemType = LLMemType::MTYPE_APPAUXRAWIMAGE;
		}

		if (!mFormattedImagep->decode(mAuxRawImage, decode_time, 4, 4))
		{
			if (!mFormattedImagep->isDecoding())
			{
				llwarns << "Failed to decode high components " << mID << ":" << gTextureTable.getName(mID) << llendl;
				abortDecode();
				destroyRawImage();
				return; // decode failed; re-request
			}
		}
		
		if (mFormattedImagep->isDecoding())
		{
			return; // Not done decoding.
		}
		
		mDecodingAux = FALSE;
	}

	if (mRawImage.notNull() && getComponents() != mRawImage->getComponents())
	{
		//
		// We've changed the number of components (presumably this is after
		// decoding the first packet of an image), so we need to move any
		// objects using this pool to a different pool.
		//
		mComponents = mRawImage->getComponents();
		gPipeline.dirtyPoolObjectTextures(this);
	}			

	//
	// We've decoded this image, and no longer need to.
	//
	setNeedsDecode(FALSE); // Done decoding image
	mIsRawImageValid = TRUE;
	
	//
	// We have a raw image, and now we need to push the data
	// from the raw image into the GL image
	//
	llassert(mRawImage.notNull());
	mNeedsCreateTexture = TRUE;

	// Everything's OK...
#if LL_DEBUG
	lldebugst(LLERR_IMAGE) << "Img: ";
	std::string tex_name = gTextureTable.getName(mID);
	if (!tex_name.empty())
	{
		llcont << tex_name;
	}
	else
	{
		llcont << mID;
	}
	llcont << " Discard level " << (S32)getDiscardLevel();
	llcont << llendl;
#endif
}

bool LLViewerImage::isDecoding()
{
	return (mFormattedImagep.notNull() && mFormattedImagep->isDecoding());
}

//============================================================================

void LLViewerImage::addTextureStats(F32 pixel_area,
								    F32 texel_area_ratio, // = 1.0
								    F32 cos_center_angle) const // = 1.0
{
	F32 virtual_size = pixel_area / texel_area_ratio;
	if (virtual_size > mMaxVirtualSize)
	{
		mMaxVirtualSize = virtual_size;
	}
	cos_center_angle = llclamp(cos_center_angle, -1.f, 1.f);
	if (cos_center_angle > mMaxCosAngle)
	{
		mMaxCosAngle = cos_center_angle;
	}
}

void LLViewerImage::resetTextureStats(BOOL zero)
{
	if (zero)
	{
		mMaxVirtualSize = 0.0f;
		mMaxCosAngle = -1.0f;
	}
	else
	{
		mMaxVirtualSize -= mMaxVirtualSize * .10f; // decay by 5%/update
		mMaxCosAngle = -1.0f;
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
	else if ((!mFullWidth && !mWidth)  || (!mFullHeight && !mHeight))
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
			// Guess the required scale factor of the image using pixels per texel..
			// Right now, use a safe 1:1 for the tradeoff - we can adjust this later.
			// Actually, it might be nice to generate a float, so we can prioritize which
			// ones we can discard quality levels from.
			discard_level = (F32)(log(mTexelsPerImage/mMaxVirtualSize) / log_4);
		}
		if (mBoostLevel < LLViewerImage::BOOST_HIGH)
		{
			static const F32 discard_bias = 0.5f; // Must be < 1 or highest discard will never load!
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

		//
		// Only need to do an actual decode if we don't have the right GL level,
		// or we don't have raw data and need raw data.
		//
		BOOL increase_discard = FALSE;
		if (getDiscardLevel() < 0 || mDesiredDiscardLevel < getDiscardLevel())
		{
			// We need to do a decode of that discard level to get more data.
			if (mFormattedImagep.notNull() && !needsDecode())
			{
				if (mFormattedImagep->getDiscardLevel() <= mDesiredDiscardLevel)
				{
					setNeedsDecode(TRUE); // processTextureStats - Changing discard level of texture
				}
				// else let the llviewerimagelist logic do its thing
			}
			else if (mFormattedFlushed)
			{
				//llinfos << "Attempting vfs reload (a) of " << mID << llendl;
				startVFSLoad();
			}
		}
		else if (sDesiredDiscardBias > 0.0f && sBoundTextureMemory > sMaxBoundTextureMem*texmem_middle_bound_scale)
		{
			// Limit the amount of GL memory bound each frame
			if (mDesiredDiscardLevel > getDiscardLevel())
			{
				increase_discard = TRUE;
			}
		}
		else if (sDesiredDiscardBias > 0.0f && sTotalTextureMemory > sTotalTextureMemory*texmem_middle_bound_scale)
		{
			// Only allow GL to have 2x the video card memory
			if (!getBoundRecently())
			{
				increase_discard = TRUE;
			}
		}
		if (increase_discard)
		{
			sBoundTextureMemory -= mTextureMemory;
			sTotalTextureMemory -= mTextureMemory;
			// Increase the discard level (reduce the texture res)
			S32 new_discard = getDiscardLevel()+1;
			setDiscardLevel(new_discard);
			sBoundTextureMemory += mTextureMemory;
			sTotalTextureMemory += mTextureMemory;
		}
	}
	
	//
	// Flush the formatted data for this image if it hasn't been used in a while.
	//
#if 1
	const F32 FLUSH_TIME = 30.f;
	if (mFormattedImagep.notNull())
	{
		if ((mLastDecodeTime.getElapsedTimeF32() > FLUSH_TIME)
			&& !mStreamFile
			&& (this->mLastPacketTimer.getElapsedTimeF32() > FLUSH_TIME)
			&& !mNeedsDecode
			&& !mFormattedImagep->isDecoding()
			&& (mDesiredDiscardLevel == getDiscardLevel()))
		{
			//llinfos << mID << ": flushing formatted image, last processed packet " << mLastPacketProcessed << llendl;
			// Treat this very similarly to like we haven't gotten any data
			// Flush all received packets that haven't been processed, they will be readded on reload.
			resetPacketData();

			setFormattedImage(NULL);
			mFullyLoaded = FALSE;

			// This flag says that we may have VFS data if we want to reload this texture
			// (or get network traffic for this texture)
			mFormattedFlushed = TRUE;
		}
	}
#endif
}

//============================================================================

F32 LLViewerImage::calcDecodePriority()
{
	F32 priority;
	S32 gldiscard = getDiscardLevel();
	S32 ddiscard = gldiscard - mDesiredDiscardLevel;

	if (mIsMissingAsset)
	{
		priority = 0.0f;
	}
	else if (mDesiredDiscardLevel > mMaxDiscardLevel)
	{
		// Don't decode anything we don't need
		priority = 0.0f;
	}
	else if (gldiscard < 0 && mDesiredDiscardLevel >= 0)
	{
		// We don't have any data yet, we need something immideately
		priority = 200000.f;
	}
	else if (getDiscardLevel() < 0 && mDesiredDiscardLevel < MAX_DISCARD_LEVEL+1)
	{
		// We have data, but haven't decoded any of it yet, but it on top
		priority = 300000.f;
	}
	else if (gldiscard <= mDesiredDiscardLevel)
	{
		priority = 0.0f;
	}
	else
	{
		// priority range = 0 - 10000 (10 ^ 4)
		if (getDontDiscard())
		{
			ddiscard+=2;
		}
		else if (!getBoundRecently())
		{
			ddiscard-=2;
		}
		else
		{
			ddiscard-=1;
		}
		ddiscard = llclamp(ddiscard, 0, 4);
		
		priority = powf(10.f,(F32)ddiscard);
	}
	if (priority > 0.0f)
	{
		F32 pixel_priority = llmin(mMaxVirtualSize * (1.5f + mMaxCosAngle) * (100.f / (1024.f*1024.f)), 100.f);
		pixel_priority = llclamp(pixel_priority, 0.0f, priority-1.f);
		priority += pixel_priority;
		if ( mBoostLevel > 0)
		{
			priority += 1000000.f + 1000.f * mBoostLevel;
		}
	}
	return priority;
}

// A value >= max value calculated above for normalization
//static
F32 LLViewerImage::maxDecodePriority()
{
	return 1400000.f;
}

void LLViewerImage::setDecodePriority(F32 priority)
{
	llassert(!mInImageList);
	if (priority < 0.0f)
	{
		mDecodePriority = calcDecodePriority();
	}
	else
	{
		mDecodePriority = priority;
	}
	if (mStreamFile)
	{
		mStreamFile->setReadPriority(priority);
	}
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

void LLViewerImage::abortDecode()
{
	// Don't try to recover, just don't set a formatted image.
	// Recovery makes the code MUCH more complex
	//llinfos << "Reset on abort decode" << llendl;
	resetPacketData();

	setFormattedImage(NULL);
	mFullyLoaded = FALSE;
	// Make sure mNeedsAux is false, otherwise it'll try to decode the 5th channel
	mNeedsAux = FALSE;
	mFullWidth = 0;
	mFullHeight = 0;
	setNeedsDecode(FALSE); // Aborting setDecodeData
	mDecodingAux = FALSE;
	if (mStreamFile)
	{
		llwarns << "Removing bad texture: " << mID << llendl;
		hoseStreamFile();
	}
	else
	{
		llwarns << "Removing bad texture: " << mID << llendl;
		LLVFile vf(gVFS, mID, LLAssetType::AT_TEXTURE, LLVFile::READ_WRITE);
		vf.remove();
	}
	mRequestedDiscardLevel = -1; // make sure we re-request the data
}

void LLViewerImage::hoseStreamFile()
{
	mStreamFile->remove();

	delete mStreamFile;
	mStreamFile = NULL;
			
	delete[] mCachedData;
	mCachedData = NULL;
	mCachedSize = 0;
}


// Sets mStreamFile to NULL when finishes loading.
BOOL LLViewerImage::loadStreamFile()
{
	LLMemType mt1(LLMemType::MTYPE_APPFMTIMAGE);
	// load as much data as possible from the stream cache file
	// TODO: unify stream cache with load local

	// are we waiting on a file read?
	if (mStreamFile)
	{
		if (mCachedSize == 0)
		{
			if (mStreamFile->isLocked(VFSLOCK_APPEND))
			{
				// avoid stalling if we are still writing to the file
				return FALSE;
			}
			mCachedSize = mStreamFile->getSize();
			if (mCachedSize >= 27)
			{
				mCachedData = new U8[mCachedSize];
				mStreamFile->read(mCachedData, mCachedSize, TRUE, 100 + mDecodePriority);
			}
			else
			{
				llwarns << "Cached image " << mID << " has length " << mCachedSize << " not loading" << llendl;

				mStreamFile->remove();

				delete mStreamFile;
				mStreamFile = NULL;
				mCachedSize = 0;
				return FALSE;
			}
		}
		
		// is it finished?
		if (mStreamFile->isReadComplete())
		{
			//llinfos << mID << ": loading from stream file " << llendl;
			U16 packet;
			U32 file_version;
			LLUUID file_id;

			U8 *tmp = mCachedData;
			memcpy(&file_version, tmp, 4);
			tmp += 4;
			memcpy(file_id.mData, tmp, 16);
			tmp += 16;

			if (file_version != sCurrentFileVersion ||
				file_id != mID)
			{
				// this file is from an old version, failed to open, or is invalid
				hoseStreamFile();

				return TRUE; // done
			}

			mGotFirstPacket = TRUE;

			memcpy(&mDataCodec, tmp, 1);
			tmp += 1;
			memcpy(&mPackets, tmp, 2);
			tmp += 2;

			memcpy(&mTotalBytes, tmp, 4);
			tmp += 4;

			while (tmp - mCachedData < mCachedSize)
			{
				memcpy(&packet, tmp, 2);
				tmp += 2;

				if (packet >= mPackets)
				{
					llwarns << "Cached image " << mID << " has bogus packet " << packet << " of " << mPackets << llendl;

					hoseStreamFile();
					return TRUE; // done
				}

				U16 data_size;
				memcpy((U8*)&(data_size), tmp, 2);
				tmp += 2;

				if (tmp + data_size > mCachedData + mCachedSize)
				{
					llwarns << "Cached image " << mID << " has bad length " << mCachedSize << ", should be " << (S32)(tmp + data_size - mCachedData) << llendl;

					hoseStreamFile();
					return TRUE; // done
				}

				if (mReceivedPacketMap.find(packet) == mReceivedPacketMap.end())
				{
					U8 *buf = new U8[data_size];
					memcpy(buf, tmp, data_size);
					mReceivedPacketMap[packet] = new LLViewerImagePacket(buf, data_size, packet, TRUE);
				}
				else
				{
					// Technically this assertion is correct, but there may be bogus VFS files out there which invalidate this
					// condition.
					//llassert(!mReceivedPacketMap[packet]->mWroteToDisk);
					mReceivedPacketMap[packet]->mWroteToDisk = TRUE;
				}

				tmp += data_size;

				mPacketsReceived++;
			}

			delete mStreamFile;
			mStreamFile = NULL;
			
			delete[] mCachedData;
			mCachedData = NULL;
 			mCachedSize = 0;

			// Make sure we process all of the packet data associated with this texture.
			mLastPacketProcessed = -1;
			mLastPacket = -1;
			mLastPacketTimer.reset();

			// Process the packets and write to disk any that have not been written
			checkPacketData();

			return TRUE; // done
		}
		else
		{
			return FALSE; // still loading
		}
	}
	else
	{
		return TRUE; // not loading
	}
}

//============================================================================

void LLViewerImage::setLoadedCallback( loaded_callback_func loaded_callback, S32 discard_level, BOOL keep_imageraw, void* userdata)
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
}

void LLViewerImage::doLoadedCallbacks()
{
	// Need to make sure we don't do these during the process of a decode or something?
	if ((mFormattedImagep.notNull() && mFormattedImagep->isDecoding()) || mNeedsCreateTexture)
	{
		return;
	}
	
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

	// assert: We should either have a valid raw image, be decoding one, or not have one at all
	llassert(mIsRawImageValid || needsDecode() || mRawImage.isNull());
	// assert: We should either not have a raw image, or it's discard level should be <= gl_discard
	llassert(!mIsRawImageValid || mRawDiscardLevel <= gl_discard);


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
		if (mFormattedImagep.notNull())
		{
			// If we don't have a raw image or a GL image, we need to decode from a formatted image
			best_aux_discard = llmin(best_aux_discard,
				mFormattedImagep->calcDiscardLevelBytes(mFormattedImagep->getDataSize()));
		}
		else
		{
			// We have no data at all, we need to get the formatted image.
			// Do this by forcing the best aux discard to be 0.
			best_aux_discard = 0;
		}
	}


	//
	// See if any of the callbacks would actually run using the data that we can provide,
	// and also determine if we need to perform any readbacks or decodes.
	//
	bool run_gl_callbacks = false;
	bool run_raw_callbacks = false;
	bool need_readback = false;
	bool need_decompress = false;

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
				else if (entryp->mLastUsedDiscard > best_aux_discard)
				{
					// We need to decompress data, but don't need
					// to run the callbacks
					need_decompress = true;
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
		createRawImage(gl_discard, TRUE);
		readBackRaw(gl_discard, mRawImage);
		mIsRawImageValid = TRUE;
	}
	if (need_decompress)
	{
		startImageDecode();
	}

	//
	// Run raw/auxiliary data callbacks
	//
	if (run_raw_callbacks && mIsRawImageValid && (mRawDiscardLevel <= mMaxDiscardLevel))
	{
		// Do callbacks which require raw image data.
		//llinfos << "doLoadedCallbacks raw for " << getID() << llendl;

		LLImageRaw* raw_image = mRawImage;
		LLImageRaw* raw_image_aux = mAuxRawImage;
		llassert(!mNeedsAux || mAuxRawImage.notNull());
		
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


				BOOL final = mRawDiscardLevel <= entryp->mDesiredDiscard ? TRUE : FALSE;
				//llinfos << "Running callback for " << getID() << llendl;
				//llinfos << raw_image->getWidth() << "x" << raw_image->getHeight() << llendl;
				if (final)
				{
					//llinfos << "Final!" << llendl;
				}
				entryp->mLastUsedDiscard = mRawDiscardLevel;
				entryp->mCallback(TRUE, this, raw_image, raw_image_aux, mRawDiscardLevel, final, entryp->mUserData);
				if (final)
				{
					iter = mLoadedCallbackList.erase(curiter);
					delete entryp;
				}
			}
		}

		//
		// If you want to keep a copy of the raw image, you better copy it off yourself
		//
		destroyRawImage();
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
}

//============================================================================

// static
void LLViewerImage::receiveImage(LLMessageSystem *msg, void **user_data)
{
	LLFastTimer t(LLFastTimer::FTM_PROCESS_IMAGES);
	
	// Receive image header, copy into image object and decompresses 
	// if this is a one-packet image. 

	LLUUID id;

	char ip_string[256];
	u32_to_ip_string(msg->getSenderIP(),ip_string);

	if (msg->getReceiveCompressedSize())
	{
		gImageList.sTextureBits += msg->getReceiveCompressedSize() * 8;
	}
	else
	{
		gImageList.sTextureBits += msg->getReceiveSize() * 8;
	}
	gImageList.sTexturePackets++;

	msg->getUUIDFast(_PREHASH_ImageID, _PREHASH_ID, id);

	char id_string[UUID_STR_LENGTH];
	id.toString(id_string);

	LLViewerImage *image = gImageList.getImage(id);					//  Look up the correct image 

	image->mLastPacketTimer.reset();
	
	if (image->mFullyLoaded)
	{
// 		llinfos << id << ":" << " Packet 0 for already loaded image!" << llendl;
		return;
	}
		
	// check to see if we've gotten this packet before
	if (image->mGotFirstPacket)
	{
		//llinfos << id << ":" << " Duplicate Packet 0" << llendl;
		return;
	}
	if (!image->mRequested)
	{
// 		llinfos << id << ":" << " Packet 0 for unrequested image!" << llendl;
		return;
	}

	image->mGotFirstPacket = TRUE;

	//	Copy header data into image object
	msg->getU8Fast(_PREHASH_ImageID, _PREHASH_Codec, image->mDataCodec);
	msg->getU16Fast(_PREHASH_ImageID, _PREHASH_Packets, image->mPackets);
	msg->getU32Fast(_PREHASH_ImageID, _PREHASH_Size, image->mTotalBytes);

	if (0 == image->mPackets)
	{
		llwarns << "Img: " << (gTextureTable.getName(id).empty() ? id_string : gTextureTable.getName(id)) << ":" << " Number of packets is 0" << llendl;
		return;
	}

	lldebugst(LLERR_IMAGE) << "Img: " << (gTextureTable.getName(id).empty() ? id_string : gTextureTable.getName(id)) << ":" << " Packet 0:" << image->mPackets - 1 << llendl;

	U16 data_size = msg->getSizeFast(_PREHASH_ImageData, _PREHASH_Data); 

	// Got a packet, reset the counter.
	image->mRequestTime.reset();
	image->mPacketsReceived++;

	if (data_size)
	{
		if (gVFS->getExists(image->mID, LLAssetType::AT_TEXTURE))
		{
			// We have data in the VFS, but it's not loaded.
			// We should start the VFS load on the assumption that we're going to use this data shortly.
			image->startVFSLoad();
			// We can throw out this data, because we have at least ONE packet on the disk
			//llinfos << "Throwing out first packet for " << image->mID << " which we already have a VFS file for!" << llendl;
			return;
		}

		// this buffer gets saved off in the packet list
		U8 *data = new U8[data_size];
		msg->getBinaryDataFast(_PREHASH_ImageData, _PREHASH_Data, data, data_size);

		// output this image data to cache file
		// only do this if we don't have old data
		LLVFile file(gVFS, image->mID, LLAssetType::AT_TEXTURE, LLVFile::APPEND);
		if (! file.getSize())
		{
			BOOL write_to_vfs = FALSE;
			if (image->mPackets == 0)
			{
				// Uh oh, we don't have packet data size
				// This must be something from a local cache
				llwarns << "Creating VFS file even though we don't know the number of packets!" << llendl;
			}
			else
			{
				write_to_vfs = file.setMaxSize(image->mTotalBytes + IMAGE_HEADER_SIZE + image->mPackets * PACKET_HEADER_SIZE);
			}

			// to avoid another dynamic allocation, just assume we won't be gettimg image packets > 1 MTU
			if (data_size > MTUBYTES)
			{
				llerrs << "image data chunk too large: " << data_size << " bytes" << llendl;
			}

			if (write_to_vfs)
			{
				const S32 WRITE_BUF_SIZE = IMAGE_HEADER_SIZE + PACKET_HEADER_SIZE + MTUBYTES;
				U8 buffer[WRITE_BUF_SIZE];
				U8 *tmp = buffer;

				// write current version byte to file, so we can change the format and detect old files later
				memcpy(tmp, &LLViewerImage::sCurrentFileVersion, 4);
				tmp += 4;
				memcpy(tmp, id.mData, 16);
				tmp += 16;
				memcpy(tmp, (U8*)&(image->mDataCodec), 1);
				tmp += 1;
				memcpy(tmp, (U8*)&(image->mPackets), 2);
				tmp += 2;
				memcpy(tmp, (U8*)&(image->mTotalBytes), 4);
				tmp += 4;
				U16 zero = 0;
				memcpy(tmp, (U8*)&zero, 2);
				tmp += 2;
				memcpy(tmp, (U8*)&(data_size), 2);
				tmp += 2;

				// now copy in the image data
				// it's a shame we can't use the original data buffer
				// but this needs to be a single, atomic write
				memcpy(tmp, data, data_size);

//				llinfos << "Writing packet 0 to disk for " << image->getID() << llendl;
				file.write(buffer, IMAGE_HEADER_SIZE + PACKET_HEADER_SIZE + data_size);
			}
			// do this AFTER writing to the VFS since LLViewerImagePacket may delete 'data'
			llassert( image->mReceivedPacketMap.find(0) == image->mReceivedPacketMap.end() );
			image->mReceivedPacketMap[0] = new LLViewerImagePacket(data, data_size, 0, TRUE);
		}

		image->checkPacketData();
	}
}


///////////////////////////////////////////////////////////////////////////////
// TODO: lastbytes vs. texturebits?
// TODO: is mRequested already used?

// static
void LLViewerImage::receiveImagePacket(LLMessageSystem *msg, void **user_data)
{
	LLMemType mt1(LLMemType::MTYPE_APPFMTIMAGE);
	LLFastTimer t(LLFastTimer::FTM_PROCESS_IMAGES);
	
	// Receives image packet, copy into image object,
	// checks if all packets received, decompresses if so. 

	LLUUID id;
	U16 packet_num;
	char id_string[UUID_STR_LENGTH];

	char ip_string[256];
	u32_to_ip_string(msg->getSenderIP(),ip_string);

	if (msg->getReceiveCompressedSize())
	{
		gImageList.sTextureBits += msg->getReceiveCompressedSize() * 8;
	}
	else
	{
		gImageList.sTextureBits += msg->getReceiveSize() * 8;
	}
	gImageList.sTexturePackets++;

	//llprintline("Start decode, image header...");
	msg->getUUIDFast(_PREHASH_ImageID, _PREHASH_ID, id);
	msg->getU16Fast(_PREHASH_ImageID, _PREHASH_Packet, packet_num);

	id.toString(id_string);
	
	LLViewerImage *image = gImageList.hasImage(id);			//  Look up the correct image 
	if (!image ||!(image->mRequested))
	{
		// Getting a packet for an unrequested image.
		lldebugst(LLERR_IMAGE) << "Img: " << (gTextureTable.getName(id).empty() ? id_string : gTextureTable.getName(id)) << " Packet ";
		llcont << packet_num << " for unrequested from " << ip_string << llendl;

		// don't cancel the request - this might just be an out of order packet
		return;
	}

	image->mLastPacketTimer.reset();
	
	if (image->mReceivedPacketMap.find(packet_num) != image->mReceivedPacketMap.end())
	{
		return;
	}

	// check to see if we already got this packet
	BOOL duplicate = FALSE;
	if (packet_num <= image->mLastPacketProcessed)
	{
		duplicate = TRUE;
	}
	else if (image->mReceivedPacketMap.find(packet_num) != image->mReceivedPacketMap.end())
	{
		duplicate = TRUE;
	}

	if (duplicate)
	{
		//llinfos << image->mID << ": duplicate packet " << packet_num << " last " << image->mLastPacketProcessed << llendl;
		return;
	}

	// Got a packet, reset the counter.
	image->mRequestTime.reset();
	image->mPacketsReceived++;

	std::string tex_name = gTextureTable.getName(id);
	if (image->mPackets == 0)
	{
		lldebugst(LLERR_IMAGE) << "Img: " << (tex_name.empty() ? id_string : tex_name) << " Packet " << packet_num << " out of order " << llendl;
	}
	else
	{
		lldebugst(LLERR_IMAGE) << "Img: " << (tex_name.empty() ? id_string : tex_name) << " Packet " << packet_num << ":" << image->mPackets - 1 << llendl;
	}


	U16 data_size = msg->getSizeFast(_PREHASH_ImageData, _PREHASH_Data); 
	if (data_size)
	{

		U8 *data = new U8[data_size];
		msg->getBinaryDataFast(_PREHASH_ImageData, _PREHASH_Data, data, data_size);

		// as above assume we won't be gettimg image packets > 1 MTU
		if (data_size > MTUBYTES)
		{
			llerrs << "image data chunk too large: " << data_size << " bytes" << llendl;
		}

		// We don't want to write it to disk yet, just put it on the queue.
		image->mReceivedPacketMap[packet_num] = new LLViewerImagePacket(data, data_size, packet_num, FALSE);
		// Process this packet.
		image->checkPacketData();
	}
}


BOOL LLViewerImage::checkPacketData()
{
	LLMemType mt1(LLMemType::MTYPE_APPFMTIMAGE);

	S32 cur_size = 0;
	S32 next_size = 0;
	S32 next_discard = llmax(getDiscardLevel()-1,0);

	// 1. Check if we already have formatted data to decode
	if (mFormattedImagep.notNull())
	{
		if ((mFormattedImagep->isDecoding()) || mStreamFile)
		{
			return TRUE; // we're busy decoding, don't request more data yet
		}
		
		cur_size = mFormattedImagep->getDataSize();
		next_size = mFormattedImagep->calcDataSize(next_discard);
		next_size = llmin(next_size, (S32)mTotalBytes);
		
		if (cur_size >= next_size)
		{
			setDecodeData(mFormattedImagep->getData(), cur_size);
			return TRUE;
		}
	}

	if (mFullyLoaded)
	{
		// Somehow we think we have new packet data but are flagged as fully loaded
		resetPacketData();
		return TRUE;
	}

	// 2. Check if we already have new packets
	if (!mPackets || !mGotFirstPacket)
	{
		return FALSE;
	}
	if (mReceivedPacketMap.empty() || mReceivedPacketMap.find(mLastPacketProcessed + 1) == mReceivedPacketMap.end())
	{
		return FALSE;
	}
	
	// 3. Decide if we have enough new data to decode
	S32 new_size = cur_size;
	U16 next_packet_num = mLastPacketProcessed;
	for (vip_map_t::iterator iter = mReceivedPacketMap.begin();
		 iter != mReceivedPacketMap.end() && iter->second->mPacketNum == ++next_packet_num;
		 iter++)
	{
		new_size += iter->second->mDataSize;
		mLastPacket = iter->second->mPacketNum;
	}
	mLastBytesProcessed = new_size;

	if (new_size < next_size)
	{
		return FALSE;
	}
 
	if (!gVFS->getExists(mID, LLAssetType::AT_TEXTURE))
	{
		// We must have removed the file, probably because it was corrupted. Abort!
		//llinfos << "Reset on no VFS file!" << llendl;
		abortDecode();
		return FALSE;
	}

	if (mLastPacketProcessed == -1 && cur_size != 0)
	{
		llerrs << "LLViewerImage: duplicate first packet!" << llendl;
	}

	// 4. Append new data to existing data and decode

	// 4a. Write the packets to disk
	LLVFile file(gVFS, mID, LLAssetType::AT_TEXTURE, LLVFile::APPEND);
	vip_map_t::iterator first_iter = mReceivedPacketMap.begin();
	S32 first_idx = 0;
	while (first_iter != mReceivedPacketMap.end())
	{
		S32 packet_data_size = 0;
		vip_map_t::iterator end_iter = first_iter;
		S32 end_idx = first_idx;
		while(end_iter != mReceivedPacketMap.end())
		{
			LLViewerImagePacket *vip = end_iter->second;
			if (!vip->mWroteToDisk)
			{
				packet_data_size += (PACKET_HEADER_SIZE + vip->mDataSize);
			}
			else
			{
				break;
			}
			++end_iter;
			++end_idx;
		}
		if (packet_data_size > 0)
		{
			U8* packet_data_buffer = new U8[packet_data_size];
			U8* packet_data = packet_data_buffer;
			for (vip_map_t::iterator iter = first_iter; iter != end_iter; ++iter)
			{
				LLViewerImagePacket *vip = iter->second;
						
				memcpy(packet_data, &vip->mPacketNum, 2);
				memcpy(packet_data + 2, &(vip->mDataSize), 2);
				memcpy(packet_data + PACKET_HEADER_SIZE, vip->mData, vip->mDataSize);
				packet_data += (PACKET_HEADER_SIZE + vip->mDataSize);
			
				vip->mWroteToDisk = TRUE;
			}
			if (packet_data - packet_data_buffer != packet_data_size) llerrs << "wtf?" << llendl;

			// 			llinfos << mID << " Writing packets " << first_idx << "-" << end_idx << " to file." << llendl;
			file.write(packet_data_buffer, packet_data_size);
		
			delete[] packet_data_buffer;
		}
		if (end_iter == first_iter)
		{
			++end_iter;
			++end_idx;
		}
		first_iter = end_iter;
		first_idx = end_idx;
	}

	// 4b. Append the data
	U8* data = new U8[new_size];
	if (cur_size > 0)
	{
		memcpy(data, mFormattedImagep->getData(), cur_size);
	}
	LLViewerImagePacket *pkt = mReceivedPacketMap.begin()->second;
	while (pkt && pkt->mPacketNum == mLastPacketProcessed + 1)
	{
		memcpy((U8*)(data + cur_size), pkt->mData, pkt->mDataSize);
		cur_size += pkt->mDataSize;
		mLastPacketProcessed = pkt->mPacketNum;
		delete mReceivedPacketMap.begin()->second;
		mReceivedPacketMap.erase(mReceivedPacketMap.begin());
		pkt = NULL;
		if (!mReceivedPacketMap.empty())
		{
			pkt = mReceivedPacketMap.begin()->second;
		}
	}

	llassert(cur_size == new_size);
	lldebugst(LLERR_IMAGE) << "IMAGE RECEIVED: " << mID.getString() << " Bytes: " << cur_size << "/" << mTotalBytes << llendl;
	
	// 4c. Set the data to be decoded, and the number of bytes to use.
	setDecodeData(data, new_size);

	// 5. Recalculate the image priority
	gImageList.removeImageFromList(this);
	F32 decode_priority = calcDecodePriority();
	setDecodePriority(decode_priority);
	gImageList.addImageToList(this);
	
	return TRUE;
}

F32 LLViewerImage::getDecodeProgress(F32 *data_progress_p)
{
	F32 decode_progress = 0.0f;
	F32 data_progress = 0.0f;
	
	if (mLastPacket >= 0)
	{
		S32 max_bytes = mTotalBytes;
		S32 data_bytes = mLastBytesProcessed;
		S32 decode_bytes = mFormattedImagep.notNull() ? mFormattedImagep->getDataSize() : data_bytes;
		data_progress = (F32)data_bytes / (F32)max_bytes;
		decode_progress = (F32)decode_bytes / (F32)max_bytes;
	}
	if (data_progress_p) *data_progress_p = data_progress;
	return decode_progress;
}

// Call with 0,0 to turn this feature off.
void LLViewerImage::setKnownDrawSize(S32 width, S32 height)
{
	mKnownDrawWidth = width;
	mKnownDrawHeight = height;
	addTextureStats((F32)(width * height));
}

// virtual
BOOL LLViewerImage::bind(S32 stage) const
{
	if (stage == -1)
	{
		return TRUE;
	}
	
	if (gNoRender)
	{
		return true;
	}
	BOOL res = bindTextureInternal(stage);
	if (res)
	{
		if (mIsMissingAsset)
		{
			// If we can bind, clearly we have an asset.
			// If mIsMissingAsset was true and we get here, it's likely
			// that the asset server was messed up and then it recovered.
			mIsMissingAsset = FALSE;
		}
	}
	else
	{
		// On failure to bind, what should we set the currently bound texture to?
		if (mIsMissingAsset && !sMissingAssetImagep.isNull() && (this != (LLImageGL *)sMissingAssetImagep))
		{
			res = sMissingAssetImagep->bind( stage );
		}
		if (!res && !sDefaultImagep.isNull() && (this != (LLImageGL *)sDefaultImagep))
		{
			// use default if we've got it
			res = sDefaultImagep->bind(stage);
		}
		if (!res && !sNullImagep.isNull() && (this != (LLImageGL *)sNullImagep))
		{
			res = sNullImagep->bind(stage);
		}
 		if (!res)
		{
			llwarns << "LLViewerImage::bindTexture failed." << llendl;
		}
		stop_glerror();
	}
	return res;
}

// Was in LLImageGL
LLImageRaw* LLViewerImage::createRawImage(S8 discard_level, BOOL allocate)
{
	llassert(discard_level >= 0);
	llassert(mFormattedImagep.isNull() || !mFormattedImagep->isDecoding());
	if (mRawImage.notNull())
	{
		llerrs << "createRawImage() called with existing mRawImage" << llendl;
		mRawImage = NULL;
		mAuxRawImage = NULL;
	}
	if (allocate && mComponents)
	{
		mRawImage = new LLImageRaw(getWidth(discard_level), getHeight(discard_level), mComponents);
		mIsRawImageValid = TRUE;
	}
	else
	{
		mRawImage = new LLImageRaw;
		mIsRawImageValid = FALSE;
	}
	mRawDiscardLevel = discard_level;
	
	return mRawImage;
}

void LLViewerImage::destroyRawImage()
{
	mRawImage = NULL;
	mAuxRawImage = NULL;
	mIsRawImageValid = FALSE;
	mRawDiscardLevel = INVALID_DISCARD_LEVEL;
}
