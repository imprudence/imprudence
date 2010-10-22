/** 
 * @file llviewerimagelist.cpp
 * @brief Object for managing the list of images within a region
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

#include "llviewerprecompiledheaders.h"

#include "llviewerimagelist.h"

#include "imageids.h"
#include "llgl.h" // fot gathering stats from GL
#include "llimagegl.h"
#include "llimagebmp.h"
#include "llimagej2c.h"
#include "llimagetga.h"
#include "llimagejpeg.h"
#include "llimagepng.h"
#include "llimageworker.h"

#include "llsdserialize.h"
#include "llsys.h"
#include "llvfs.h"
#include "llvfile.h"
#include "llvfsthread.h"
#include "llxmltree.h"
#include "message.h"

#include "llagent.h"
#include "lltexturecache.h"
#include "lltexturefetch.h"
#include "llviewercontrol.h"
#include "llviewerimage.h"
#include "llviewermedia.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "pipeline.h"
#include "llappviewer.h"

#include <sys/stat.h>

#ifdef LL_DARWIN
#include "llwindowmacosx-objc.h"
#endif

////////////////////////////////////////////////////////////////////////////

void (*LLViewerImageList::sUUIDCallback)(void **, const LLUUID&) = NULL;

U32 LLViewerImageList::sTextureBits = 0;
U32 LLViewerImageList::sTexturePackets = 0;

LLViewerImageList gImageList;

S32 LLViewerImageList::sNumImages = 0;
LLStat LLViewerImageList::sNumImagesStat(32, TRUE);
LLStat LLViewerImageList::sNumRawImagesStat(32, TRUE);
LLStat LLViewerImageList::sGLTexMemStat(32, TRUE);
LLStat LLViewerImageList::sGLBoundMemStat(32, TRUE);
LLStat LLViewerImageList::sRawMemStat(32, TRUE);
LLStat LLViewerImageList::sFormattedMemStat(32, TRUE);

///////////////////////////////////////////////////////////////////////////////

LLViewerImageList::LLViewerImageList() 
	: mForceResetTextureStats(FALSE),
	mUpdateStats(FALSE),
	mMaxResidentTexMemInMegaBytes(0),
	mMaxTotalTextureMemInMegaBytes(0)
{
}

void LLViewerImageList::init()
{
	sNumImages = 0;
	mMaxResidentTexMemInMegaBytes = 0;
	mMaxTotalTextureMemInMegaBytes = 0 ;
	
	if (gNoRender)
	{
		// Don't initialize GL stuff if we're not rendering.
		return;
	}
	
	mUpdateStats = TRUE;
	
	// Update how much texture RAM we're allowed to use.
	updateMaxResidentTexMem(0); // 0 = use current
	
	doPreloadImages();
}


void LLViewerImageList::doPreloadImages()
{
	LL_DEBUGS("ViewerImages") << "Preloading images..." << LL_ENDL;
	
	// Set the "missing asset" image
	LLViewerImage::sMissingAssetImagep = getImageFromFile("missing_asset.tga", MIPMAP_NO, IMMEDIATE_YES);
	
	// Set the "white" image
	LLViewerImage::sWhiteImagep = getImageFromFile("white.tga", MIPMAP_NO, IMMEDIATE_YES);
	
	LLUIImageList* image_list = LLUIImageList::getInstance();

	image_list->initFromFile();
	
	// turn off clamping and bilinear filtering for uv picking images
	//LLViewerImage* uv_test = preloadUIImage("uv_test1.tga", LLUUID::null, FALSE);
	//uv_test->setClamp(FALSE, FALSE);
	//uv_test->setMipFilterNearest(TRUE, TRUE);
	//uv_test = preloadUIImage("uv_test2.tga", LLUUID::null, FALSE);
	//uv_test->setClamp(FALSE, FALSE);
	//uv_test->setMipFilterNearest(TRUE, TRUE);

	// prefetch specific UUIDs
	gImageList.getImageFromFile(IMG_SHOT.asString()+".j2c", TRUE, TRUE); /*getImage(IMG_SHOT, TRUE);*/
	gImageList.getImageFromFile(IMG_SMOKE_POOF.asString()+".j2c", TRUE, TRUE); /*getImage(IMG_SMOKE_POOF, TRUE);*/
	LLViewerImage* image = getImageFromFile("silhouette.j2c", MIPMAP_YES, IMMEDIATE_YES);
	if (image) 
	{
		image->setAddressMode(LLTexUnit::TAM_WRAP);
		mImagePreloads.insert(image);
	}
	image = getImageFromFile("noentrylines.j2c", MIPMAP_YES, IMMEDIATE_YES);
	if (image) 
	{
		image->setAddressMode(LLTexUnit::TAM_WRAP);	
		mImagePreloads.insert(image);
	}
	image = getImageFromFile("noentrypasslines.j2c", MIPMAP_YES, IMMEDIATE_YES);
	if (image) 
	{
		image->setAddressMode(LLTexUnit::TAM_WRAP);
		mImagePreloads.insert(image);
	}
	image = getImageFromFile(DEFAULT_WATER_NORMAL.asString()+".j2c", MIPMAP_YES, IMMEDIATE_YES,0,0,DEFAULT_WATER_NORMAL);
	if (image) 
	{
		image->setAddressMode(LLTexUnit::TAM_WRAP);	
		mImagePreloads.insert(image);
	}
	image = getImageFromFile("8dcd4a48-2d37-4909-9f78-f7a9eb4ef903.j2c", MIPMAP_YES, IMMEDIATE_YES,0,0,LLUUID("8dcd4a48-2d37-4909-9f78-f7a9eb4ef903"));
	if (image) 
	{
		image->setAddressMode(LLTexUnit::TAM_WRAP);
		mImagePreloads.insert(image);
	}

	std::string id;

	// Preload default avatar eyes
	id = gSavedSettings.getString("UIImgDefaultEyesUUID");
	image = getImageFromFile(id+".j2c",MIPMAP_YES,IMMEDIATE_YES,0,0,LLUUID(id));
	if (image)
	{
		image->setAddressMode(LLTexUnit::TAM_WRAP);
		mImagePreloads.insert(image);
	}

	// Preload default avatar hair
	id = gSavedSettings.getString("UIImgDefaultHairUUID");
	image = getImageFromFile(id+".j2c",MIPMAP_YES,IMMEDIATE_YES,0,0,LLUUID(id));
	if (image)
	{
		image->setAddressMode(LLTexUnit::TAM_WRAP);
		mImagePreloads.insert(image);
	}

}

static std::string get_texture_list_name()
{
	BOOL login_last = gSavedSettings.getBOOL("LoginLastLocation");
	return std::string("texture_list_") + (login_last?"last":"home") + ".xml";
}

void LLViewerImageList::doPrefetchImages()
{
#if 1
    if (LLAppViewer::instance()->getPurgeCache())
	{
		// cache was purged, no point
		return;
	}
	
	// Pre-fetch textures from last logout
	LLSD imagelist;
	std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, get_texture_list_name());
	llifstream file;
	file.open(filename);
	if (file.is_open())
	{
		LLSDSerialize::fromXML(imagelist, file);
	}
	for (LLSD::array_iterator iter = imagelist.beginArray();
		 iter != imagelist.endArray(); ++iter)
	{
		LLSD imagesd = *iter;
		LLUUID uuid = imagesd["uuid"];
		S32 pixel_area = imagesd["area"];
		LLViewerImage* image = getImage(uuid, MIPMAP_TRUE, FALSE);
		if (image)
		{
			image->addTextureStats((F32)pixel_area);
		}
	}
#endif
	
}

///////////////////////////////////////////////////////////////////////////////

LLViewerImageList::~LLViewerImageList()
{
}

void LLViewerImageList::shutdown()
{
	// clear out preloads
	mImagePreloads.clear();

	// Write out list of currently loaded textures for precaching on startup
	typedef std::set<std::pair<S32,LLViewerImage*> > image_area_list_t;
	image_area_list_t image_area_list;
	for (image_priority_list_t::iterator iter = mImageList.begin();
		 iter != mImageList.end(); ++iter)
	{
		LLViewerImage* image = *iter;
		if (!image->getUseDiscard() ||
			image->needsAux() ||
			image->getTargetHost() != LLHost::invalid)
		{
			continue; // avoid UI, baked, and other special images
		}
		if(!image->getBoundRecently())
		{
			continue ;
		}
		S32 desired = image->getDesiredDiscardLevel();
		if (desired >= 0 && desired < MAX_DISCARD_LEVEL)
		{
			S32 pixel_area = image->getWidth(desired) * image->getHeight(desired);
			image_area_list.insert(std::make_pair(pixel_area, image));
		}
	}
	
	LLSD imagelist;
	const S32 max_count = 1000;
	S32 count = 0;
	for (image_area_list_t::reverse_iterator riter = image_area_list.rbegin();
		 riter != image_area_list.rend(); ++riter)
	{
		LLViewerImage* image = riter->second;
		imagelist[count]["area"] = riter->first;
		imagelist[count]["uuid"] = image->getID();
		if (++count >= max_count)
			break;
	}
	
	if (count > 0 && !gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "").empty())
	{
		std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, get_texture_list_name());
		llofstream file;
		file.open(filename);
		LLSDSerialize::toPrettyXML(imagelist, file);
	}
	
	//
	// Clean up "loaded" callbacks.
	//
	mCallbackList.clear();
	
	// Flush all of the references
	mLoadingStreamList.clear();
	mCreateTextureList.clear();
	
	mUUIDMap.clear();
	
	mImageList.clear();
}

void LLViewerImageList::dump()
{
	llinfos << "LLViewerImageList::dump()" << llendl;
	for (image_priority_list_t::iterator it = mImageList.begin(); it != mImageList.end(); ++it)
	{
		LLViewerImage* image = *it;
		
		llinfos << "priority " << image->getDecodePriority()
		<< " boost " << image->getBoostLevel()
		<< " size " << image->getWidth() << "x" << image->getHeight()
		<< " discard " << image->getDiscardLevel()
		<< " desired " << image->getDesiredDiscardLevel()
		<< " http://asset.siva.lindenlab.com/" << image->getID() << ".texture"
		<< llendl;
	}
}

void LLViewerImageList::destroyGL(BOOL save_state)
{
	LLImageGL::destroyGL(save_state);
}

void LLViewerImageList::restoreGL()
{
	LLImageGL::restoreGL();
}

/* Vertical tab container button image IDs
 Seem to not decode when running app in debug.
 
 const LLUUID BAD_IMG_ONE("1097dcb3-aef9-8152-f471-431d840ea89e");
 const LLUUID BAD_IMG_TWO("bea77041-5835-1661-f298-47e2d32b7a70");
 */

///////////////////////////////////////////////////////////////////////////////

LLViewerImage* LLViewerImageList::getImageFromFile(const std::string& filename,
												   BOOL usemipmaps,
												   BOOL level_immediate,
												   LLGLint internal_format,
												   LLGLenum primary_format, 
												   const LLUUID& force_id)
{
	std::string full_path = gDirUtilp->findSkinnedFilename("textures", filename);
	if (full_path.empty())
	{
		llwarns << "Failed to find local image file: " << filename << llendl;
		return getImage(IMG_DEFAULT, TRUE, TRUE);
	}

	std::string url = "file://" + full_path;

	return getImageFromUrl(url, usemipmaps, level_immediate, internal_format, primary_format, force_id);
}

LLViewerImage* LLViewerImageList::getImageFromUrl(const std::string& url,
												   BOOL usemipmaps,
												   BOOL level_immediate,
												   LLGLint internal_format,
												   LLGLenum primary_format, 
												   const LLUUID& force_id)
{
	if (gNoRender)
	{
		// Never mind that this ignores image_set_id;
		// getImage() will handle that later.
		return getImage(IMG_DEFAULT, TRUE, TRUE);
	}

	// generate UUID based on hash of filename
	LLUUID new_id;
	if (force_id.notNull())
	{
		new_id = force_id;
	}
	else
	{
		new_id.generate(url);
	}

	LLPointer<LLViewerImage> imagep = hasImage(new_id);
	
	if (imagep.isNull())
	{
		imagep = new LLViewerImage(url, new_id, usemipmaps);
		
		if (internal_format && primary_format)
		{
			imagep->setExplicitFormat(internal_format, primary_format);
		}

		addImage(imagep);
		
		if (level_immediate)
		{
			imagep->dontDiscard();
			imagep->setBoostLevel(LLViewerImageBoostLevel::BOOST_UI);
		}
	}

	imagep->setGLTextureCreated(true);

	return imagep;
}


LLViewerImage* LLViewerImageList::getImage(const LLUUID &image_id,
												   BOOL usemipmaps,
												   BOOL level_immediate,
												   LLGLint internal_format,
												   LLGLenum primary_format,
												   LLHost request_from_host)
{
	// Return the image with ID image_id
	// If the image is not found, creates new image and
	// enqueues a request for transmission
	
	if ((&image_id == NULL) || image_id.isNull())
	{
		return (getImage(IMG_DEFAULT, TRUE, TRUE));
	}
	
	LLPointer<LLViewerImage> imagep = hasImage(image_id);
	
	if (imagep.isNull())
	{
		imagep = new LLViewerImage(image_id, request_from_host, usemipmaps);
		
		if (internal_format && primary_format)
		{
			imagep->setExplicitFormat(internal_format, primary_format);
		}
		
		addImage(imagep);
		
		if (level_immediate)
		{
			imagep->dontDiscard();
			imagep->setBoostLevel(LLViewerImageBoostLevel::BOOST_UI);
		}
		else
		{
			//by default, the texure can not be removed from memory even if it is not used.
			//here turn this off
			//if this texture should be set to NO_DELETE, either pass level_immediate == TRUE here, or call setNoDelete() afterwards.
			imagep->forceActive() ;
		}
	}

	imagep->setGLTextureCreated(true);
	
	return imagep;
}

LLViewerImage *LLViewerImageList::hasImage(const LLUUID &image_id)
{
	uuid_map_t::iterator iter = mUUIDMap.find(image_id);
	if(iter == mUUIDMap.end())
		return NULL;
	return iter->second;
}

void LLViewerImageList::addImageToList(LLViewerImage *image)
{
	llassert(image);
	if (image->mInImageList)
	{
		llerrs << "LLViewerImageList::addImageToList - Image already in list" << llendl;
	}
	llverify((mImageList.insert(image)).second == true);
	image->mInImageList = TRUE;
}

void LLViewerImageList::removeImageFromList(LLViewerImage *image)
{
	llassert(image);
	if (!image->mInImageList)
	{
		llinfos << "RefCount: " << image->getNumRefs() << llendl ;
		uuid_map_t::iterator iter = mUUIDMap.find(image->getID());
		if(iter == mUUIDMap.end() || iter->second != image)
		{
			llinfos << "Image is not in mUUIDMap!" << llendl ;
		}
		llerrs << "LLViewerImageList::removeImageFromList - Image not in list" << llendl;
	}
	llverify(mImageList.erase(image) == 1);
	image->mInImageList = FALSE;
}

void LLViewerImageList::addImage(LLViewerImage *new_image)
{
	if (!new_image)
	{
		llwarning("No image to add to image list", 0);
		return;
	}
	LLUUID image_id = new_image->getID();
	
	LLViewerImage *image = hasImage(image_id);
	if (image)
	{
		llwarns << "Image with ID " << image_id << " already in list" << llendl;
	}
	sNumImages++;
	
	addImageToList(new_image);
	mUUIDMap[image_id] = new_image;
}


void LLViewerImageList::deleteImage(LLViewerImage *image)
{
	if( image)
	{
		if (image->hasCallbacks())
		{
			mCallbackList.erase((LLViewerImage*)image);
		}

		llverify(mUUIDMap.erase(image->getID()) == 1);
		sNumImages--;
		removeImageFromList(image);
	}
}

///////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////

void LLViewerImageList::dirtyImage(LLViewerImage *image)
{
	mDirtyTextureList.insert(image);
}

////////////////////////////////////////////////////////////////////////////

void LLViewerImageList::updateImages(F32 max_time)
{
	llpushcallstacks ;
	LLAppViewer::getTextureFetch()->setTextureBandwidth(LLViewerStats::getInstance()->mTextureKBitStat.getMeanPerSec());

	sNumImagesStat.addValue(sNumImages);
	sNumRawImagesStat.addValue(LLImageRaw::sRawImageCount);
	sGLTexMemStat.addValue((F32)BYTES_TO_MEGA_BYTES(LLImageGL::sGlobalTextureMemoryInBytes));
	sGLBoundMemStat.addValue((F32)BYTES_TO_MEGA_BYTES(LLImageGL::sBoundTextureMemoryInBytes));
	sRawMemStat.addValue((F32)BYTES_TO_MEGA_BYTES(LLImageRaw::sGlobalRawMemory));
	sFormattedMemStat.addValue((F32)BYTES_TO_MEGA_BYTES(LLImageFormatted::sGlobalFormattedMemory));

	llpushcallstacks ;

	updateImagesDecodePriorities();

	llpushcallstacks ;
	F32 total_max_time = max_time;
	max_time -= updateImagesFetchTextures(max_time);

	llpushcallstacks ;
	max_time = llmax(max_time, total_max_time*.25f); // at least 25% of max_time
	max_time -= updateImagesCreateTextures(max_time);
	
	llpushcallstacks ;
	
	if (!mDirtyTextureList.empty())
	{
		LLFastTimer t(LLFastTimer::FTM_IMAGE_MARK_DIRTY);
		gPipeline.dirtyPoolObjectTextures(mDirtyTextureList);
		mDirtyTextureList.clear();
	}
	llpushcallstacks ;
	bool didone = false;
	for (image_list_t::iterator iter = mCallbackList.begin();
		iter != mCallbackList.end(); )
	{
		//trigger loaded callbacks on local textures immediately
		LLViewerImage* image = *iter++;
		if (!image->mUrl.empty())
		{
			// Do stuff to handle callbacks, update priorities, etc.
			didone = image->doLoadedCallbacks();
		}
		else if (!didone)
		{
			// Do stuff to handle callbacks, update priorities, etc.
			didone = image->doLoadedCallbacks();
		}
	}
	llpushcallstacks ;
	if (!gNoRender && !gGLManager.mIsDisabled)
	{
		LLViewerMedia::updateMedia();
	}
	llpushcallstacks ;
	updateImagesUpdateStats();
	llpushcallstacks ;
}

void LLViewerImageList::updateImagesDecodePriorities()
{
	// Update the decode priority for N images each frame
	{
		const size_t max_update_count = llmin((S32) (1024*gFrameIntervalSeconds) + 1, 32); //target 1024 textures per second
		S32 update_counter = llmin(max_update_count, mUUIDMap.size()/10);
		uuid_map_t::iterator iter = mUUIDMap.upper_bound(mLastUpdateUUID);
		while(update_counter > 0 && !mUUIDMap.empty())
		{
			if (iter == mUUIDMap.end())
			{
				iter = mUUIDMap.begin();
			}
			mLastUpdateUUID = iter->first;
			LLPointer<LLViewerImage> imagep = iter->second;
			++iter; // safe to incrament now

			//
			// Flush formatted images using a lazy flush
			//
			const F32 LAZY_FLUSH_TIMEOUT = 30.f; // stop decoding
			const F32 MAX_INACTIVE_TIME  = 50.f; // actually delete
			S32 min_refs = 3; // 1 for mImageList, 1 for mUUIDMap, 1 for local reference
			if (imagep->hasCallbacks())
			{
				min_refs++; // Add an extra reference if we're on the loaded callback list
			}
			S32 num_refs = imagep->getNumRefs();
			if (num_refs == min_refs)
			{
				if (imagep->mLastReferencedTimer.getElapsedTimeF32() > LAZY_FLUSH_TIMEOUT)
				{
					// Remove the unused image from the image list
					deleteImage(imagep);
					imagep = NULL; // should destroy the image								
				}
				continue;
			}
			else
			{
				if(imagep->isDeleted())
				{
					continue ;
				}
				else if(imagep->isDeletionCandidate())
				{
					imagep->destroyTexture() ;																
					continue ;
				}
				else if(imagep->isInactive())
				{
					if (imagep->mLastReferencedTimer.getElapsedTimeF32() > MAX_INACTIVE_TIME)
					{
						imagep->setDeletionCandidate() ;
					}
					continue ;
				}
				else
				{
					imagep->mLastReferencedTimer.reset();

					//reset texture state.
					imagep->setInactive() ;										
				}
			}

			imagep->processTextureStats();
			F32 old_priority = imagep->getDecodePriority();
			F32 old_priority_test = llmax(old_priority, 0.0f);
			F32 decode_priority = imagep->calcDecodePriority();
			F32 decode_priority_test = llmax(decode_priority, 0.0f);
			// Ignore < 20% difference
			if ((decode_priority_test < old_priority_test * .8f) ||
				(decode_priority_test > old_priority_test * 1.25f))
			{
				removeImageFromList(imagep);
				imagep->setDecodePriority(decode_priority);
				addImageToList(imagep);
			}
			update_counter--;
		}
	}
}

/*
 static U8 get_image_type(LLViewerImage* imagep, LLHost target_host)
 {
 // Having a target host implies this is a baked image.  I don't
 // believe that boost level has been set at this point. JC
 U8 type_from_host = (target_host.isOk() 
 ? LLImageBase::TYPE_AVATAR_BAKE 
 : LLImageBase::TYPE_NORMAL);
 S32 boost_level = imagep->getBoostLevel();
 U8 type_from_boost = ( (boost_level == LLViewerImage::BOOST_AVATAR_BAKED 
 || boost_level == LLViewerImage::BOOST_AVATAR_BAKED_SELF)
 ? LLImageBase::TYPE_AVATAR_BAKE 
 : LLImageBase::TYPE_NORMAL);
 if (type_from_host == LLImageBase::TYPE_NORMAL
 && type_from_boost == LLImageBase::TYPE_AVATAR_BAKE)
 {
 llwarns << "TAT: get_image_type() type_from_host doesn't match type_from_boost"
 << " host " << target_host
 << " boost " << imagep->getBoostLevel()
 << " imageid " << imagep->getID()
 << llendl;
 imagep->dump();
 }
 return type_from_host;
 }
 */

F32 LLViewerImageList::updateImagesCreateTextures(F32 max_time)
{
	if (gNoRender || gGLManager.mIsDisabled) return 0.0f;
	
	//
	// Create GL textures for all textures that need them (images which have been
	// decoded, but haven't been pushed into GL).
	//
	LLFastTimer t(LLFastTimer::FTM_IMAGE_CREATE);
	
	LLTimer create_timer;
	image_list_t::iterator enditer = mCreateTextureList.begin();
	for (image_list_t::iterator iter = mCreateTextureList.begin();
		 iter != mCreateTextureList.end();)
	{
		image_list_t::iterator curiter = iter++;
		enditer = iter;
		LLViewerImage *imagep = *curiter;
		imagep->createTexture();
		if (create_timer.getElapsedTimeF32() > max_time)
		{
			break;
		}
	}
	mCreateTextureList.erase(mCreateTextureList.begin(), enditer);
	return create_timer.getElapsedTimeF32();
}

void LLViewerImageList::bumpToMaxDecodePriority(LLViewerImage* imagep)
{
	if(!imagep)
	{
		return ;
	}
	if(imagep->mInImageList)
	{
		if (imagep->getDecodePriority() == LLViewerImage::maxDecodePriority())
		{
			// Already at maximum.
		  	return;
		}
		removeImageFromList(imagep);
	}

	imagep->processTextureStats();
	F32 decode_priority = LLViewerImage::maxDecodePriority() ;
	imagep->setDecodePriority(decode_priority);
	mImageList.insert(imagep);
	imagep->mInImageList = TRUE;

	return ;
}

F32 LLViewerImageList::updateImagesFetchTextures(F32 max_time)
{
	LLTimer image_op_timer;
	
	// Update the decode priority for N images each frame
	// Make a list with 32 high priority entries + 256 cycled entries
	const size_t max_priority_count = llmin((S32) (256*10.f*gFrameIntervalSeconds)+1, 32);
	const size_t max_update_count = llmin((S32) (1024*10.f*gFrameIntervalSeconds)+1, 256);
	
	// 32 high priority entries
	typedef std::vector<LLViewerImage*> entries_list_t;
	entries_list_t entries;
	size_t update_counter = llmin(max_priority_count, mImageList.size());
	image_priority_list_t::iterator iter1 = mImageList.begin();
	while(update_counter > 0)
	{
		entries.push_back(*iter1);
		++iter1;
		update_counter--;
	}
	
	// 256 cycled entries
	update_counter = llmin(max_update_count, mUUIDMap.size());
	if (update_counter > 0)
	{
		uuid_map_t::iterator iter2 = mUUIDMap.upper_bound(mLastFetchUUID);
		uuid_map_t::iterator iter2p = iter2;
		while(update_counter > 0)
		{
			if (iter2 == mUUIDMap.end())
			{
				iter2 = mUUIDMap.begin();
			}
			entries.push_back(iter2->second);
			iter2p = iter2++;
			update_counter--;
		}
		mLastFetchUUID = iter2p->first;
	}
	
	S32 fetch_count = 0;
	S32 min_count = max_priority_count + max_update_count/4;
	for (entries_list_t::iterator iter3 = entries.begin();
		 iter3 != entries.end(); )
	{
		LLPointer<LLViewerImage> imagep = *iter3++;
		
		bool fetching = imagep->updateFetch();
		if (fetching)
		{
			fetch_count++;
		}
		if (min_count <= 0 && image_op_timer.getElapsedTimeF32() > max_time)
		{
			break;
		}
		min_count--;
	}
	if (fetch_count == 0)
	{
		gDebugTimers[0].pause();
	}
	else
	{
		gDebugTimers[0].unpause();
	}
	
	
	return image_op_timer.getElapsedTimeF32();
}

void LLViewerImageList::updateImagesUpdateStats()
{
	if (mUpdateStats && mForceResetTextureStats)
	{
		for (image_priority_list_t::iterator iter = mImageList.begin();
			 iter != mImageList.end(); )
		{
			LLViewerImage* imagep = *iter++;
			imagep->resetTextureStats();
		}
		mUpdateStats = FALSE;
		mForceResetTextureStats = FALSE;
	}
}

void LLViewerImageList::decodeAllImages(F32 max_time)
{
	LLTimer timer;
	if(gNoRender) return;
	
	// Update texture stats and priorities
	std::vector<LLPointer<LLViewerImage> > image_list;
	for (image_priority_list_t::iterator iter = mImageList.begin();
		 iter != mImageList.end(); )
	{
		LLViewerImage* imagep = *iter++;
		image_list.push_back(imagep);
		imagep->mInImageList = FALSE;
	}
	mImageList.clear();
	for (std::vector<LLPointer<LLViewerImage> >::iterator iter = image_list.begin();
		 iter != image_list.end(); ++iter)
	{
		LLViewerImage* imagep = *iter;
		imagep->processTextureStats();
		F32 decode_priority = imagep->calcDecodePriority();
		imagep->setDecodePriority(decode_priority);
		mImageList.insert(imagep);
		imagep->mInImageList = TRUE;
	}
	image_list.clear();
	
	// Update fetch (decode)
	for (image_priority_list_t::iterator iter = mImageList.begin();
		 iter != mImageList.end(); )
	{
		LLViewerImage* imagep = *iter++;
		imagep->updateFetch();
	}
	// Run threads
	S32 fetch_pending = 0;
	while (1)
	{
		LLAppViewer::instance()->getTextureCache()->update(1); // unpauses the texture cache thread
		LLAppViewer::instance()->getImageDecodeThread()->update(1); // unpauses the image thread
		fetch_pending = LLAppViewer::instance()->getTextureFetch()->update(1); // unpauses the texture fetch thread
		if (fetch_pending == 0 || timer.getElapsedTimeF32() > max_time)
		{
			break;
		}
	}
	// Update fetch again
	for (image_priority_list_t::iterator iter = mImageList.begin();
		 iter != mImageList.end(); )
	{
		LLViewerImage* imagep = *iter++;
		imagep->updateFetch();
	}
	max_time -= timer.getElapsedTimeF32();
	max_time = llmax(max_time, .001f);
	F32 create_time = updateImagesCreateTextures(max_time);
	
	LL_DEBUGS("ViewerImages") << "decodeAllImages() took " << timer.getElapsedTimeF32() << " seconds. " 
	<< " fetch_pending " << fetch_pending
	<< " create_time " << create_time
	<< LL_ENDL;
}


BOOL LLViewerImageList::createUploadFile(const std::string& filename,
										 const std::string& out_filename,
										 const U8 codec)
{
	// First, load the image.
	LLPointer<LLImageRaw> raw_image = new LLImageRaw;
#ifdef LL_DARWIN
	if (!decodeImageQuartz(filename, raw_image))
		return FALSE;
#else
	switch (codec)
	{
		case IMG_CODEC_BMP:
		{
			LLPointer<LLImageBMP> bmp_image = new LLImageBMP;
			
			if (!bmp_image->load(filename))
			{
				return FALSE;
			}
			
			if (!bmp_image->decode(raw_image, 0.0f))
			{
				return FALSE;
			}
		}
			break;
		case IMG_CODEC_TGA:
		{
			LLPointer<LLImageTGA> tga_image = new LLImageTGA;
			
			if (!tga_image->load(filename))
			{
				return FALSE;
			}
			
			if (!tga_image->decode(raw_image))
			{
				return FALSE;
			}
			
			if(	(tga_image->getComponents() != 3) &&
			   (tga_image->getComponents() != 4) )
			{
				tga_image->setLastError( "Image files with less than 3 or more than 4 components are not supported." );
				return FALSE;
			}
		}
			break;
		case IMG_CODEC_JPEG:
		{
			LLPointer<LLImageJPEG> jpeg_image = new LLImageJPEG;
			
			if (!jpeg_image->load(filename))
			{
				return FALSE;
			}
			
			if (!jpeg_image->decode(raw_image, 0.0f))
			{
				return FALSE;
			}
		}
			break;
		case IMG_CODEC_PNG:
		{
			LLPointer<LLImagePNG> png_image = new LLImagePNG;
			
			if (!png_image->load(filename))
			{
				return FALSE;
			}
			
			if (!png_image->decode(raw_image, 0.0f))
			{
				return FALSE;
			}
		}
			break;
		default:
			return FALSE;
	}
#endif
	LLPointer<LLImageJ2C> compressedImage = convertToUploadFile(raw_image);
	
	if( !compressedImage->save(out_filename) )
	{
		llinfos << "Couldn't create output file " << out_filename << llendl;
		return FALSE;
	}
	
	// test to see if the encode and save worked.
	LLPointer<LLImageJ2C> integrity_test = new LLImageJ2C;
	if( !integrity_test->loadAndValidate( out_filename ) )
	{
		llinfos << "Image: " << out_filename << " is corrupt." << llendl;
		return FALSE;
	}
	
	return TRUE;
}

// note: modifies the argument raw_image!!!!
LLPointer<LLImageJ2C> LLViewerImageList::convertToUploadFile(LLPointer<LLImageRaw> raw_image)
{
	raw_image->biasedScaleToPowerOfTwo(LLViewerImage::MAX_IMAGE_SIZE_DEFAULT);
	LLPointer<LLImageJ2C> compressedImage = new LLImageJ2C();
	compressedImage->setRate(0.f);
	
	if (gSavedSettings.getBOOL("LosslessJ2CUpload") &&
		(raw_image->getWidth() * raw_image->getHeight() <= LL_IMAGE_REZ_LOSSLESS_CUTOFF * LL_IMAGE_REZ_LOSSLESS_CUTOFF))
		compressedImage->setReversible(TRUE);
	
	compressedImage->encode(raw_image, 0.0f);
	
	return compressedImage;
}
	
// Returns min setting for TextureMemory (in MB)
S32 LLViewerImageList::getMinVideoRamSetting()
{
	S32 system_ram = (S32)BYTES_TO_MEGA_BYTES(gSysMemory.getPhysicalMemoryClamped());
	//min texture mem sets to 64M if total physical mem is more than 1.5GB
	return (system_ram > 1500) ? 64 : MIN_VIDEO_RAM_IN_MEGA_BYTES ;
}

//static
// Returns max setting for TextureMemory (in MB)
S32 LLViewerImageList::getMaxVideoRamSetting(bool get_recommended)
{
	S32 max_texmem;
	if (gGLManager.mVRAM != 0)
	{
		// Treat any card with < 32 MB (shudder) as having 32 MB
		//  - it's going to be swapping constantly regardless
		S32 max_vram = gGLManager.mVRAM;
		max_vram = llmax(max_vram, getMinVideoRamSetting());
		max_texmem = max_vram;
		if (!get_recommended)
			max_texmem *= 2;
	}
	else
	{
		if (get_recommended)
			max_texmem = 128;
		else
			max_texmem = 512;
		llwarns << "VRAM amount not detected, defaulting to " << max_texmem << " MB" << llendl;
	}

	S32 system_ram = (S32)BYTES_TO_MEGA_BYTES(gSysMemory.getPhysicalMemoryClamped()); // In MB
	//llinfos << "*** DETECTED " << system_ram << " MB of system memory." << llendl;
	if (get_recommended)
		max_texmem = llmin(max_texmem, (S32)(system_ram/2));
	else
		max_texmem = llmin(max_texmem, (S32)(system_ram));
		
	max_texmem = llclamp(max_texmem, getMinVideoRamSetting(), MAX_VIDEO_RAM_IN_MEGA_BYTES); 
	
	return max_texmem;
}

const S32 VIDEO_CARD_FRAMEBUFFER_MEM = 12; // MB

void LLViewerImageList::updateMaxResidentTexMem(S32 mem)
{
	// Initialize the image pipeline VRAM settings
	S32 cur_mem = gSavedSettings.getS32("TextureMemory");
	F32 mem_multiplier = gSavedSettings.getF32("RenderTextureMemoryMultiple");
	S32 default_mem = getMaxVideoRamSetting(true); // recommended default
	if (mem == 0)
	{
		mem = cur_mem > 0 ? cur_mem : default_mem;
	}
	else if (mem < 0)
	{
		mem = default_mem;
	}

	// limit the texture memory to a multiple of the default if we've found some cards to behave poorly otherwise
	mem = llmin(mem, (S32) (mem_multiplier * (F32) default_mem));

	mem = llclamp(mem, getMinVideoRamSetting(), getMaxVideoRamSetting());
	if (mem != cur_mem)
	{
		gSavedSettings.setS32("TextureMemory", mem);
		return; //listener will re-enter this function
	}

	// TODO: set available resident texture mem based on use by other subsystems
	// currently max(12MB, VRAM/4) assumed...
	
	S32 vb_mem = mem;
	S32 fb_mem = llmax(VIDEO_CARD_FRAMEBUFFER_MEM, vb_mem/4);
	mMaxResidentTexMemInMegaBytes = (vb_mem - fb_mem) ; //in MB
	
	mMaxTotalTextureMemInMegaBytes = mMaxResidentTexMemInMegaBytes * 2;
	if (mMaxResidentTexMemInMegaBytes > 640)
	{
		mMaxTotalTextureMemInMegaBytes -= (mMaxResidentTexMemInMegaBytes >> 2);
	}
	
	if (mMaxTotalTextureMemInMegaBytes > (S32)BYTES_TO_MEGA_BYTES(gSysMemory.getPhysicalMemoryClamped()) - 128)
	{
		mMaxTotalTextureMemInMegaBytes = (S32)BYTES_TO_MEGA_BYTES(gSysMemory.getPhysicalMemoryClamped()) - 128 ;
	}
	
	llinfos << "Total Video Memory set to: " << vb_mem << " MB" << llendl;
	llinfos << "Available Texture Memory set to: " << (vb_mem - fb_mem) << " MB" << llendl;
}

///////////////////////////////////////////////////////////////////////////////

// static
void LLViewerImageList::receiveImageHeader(LLMessageSystem *msg, void **user_data)
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
	
	U8 codec;
	U16 packets;
	U32 totalbytes;
	msg->getUUIDFast(_PREHASH_ImageID, _PREHASH_ID, id);
	msg->getU8Fast(_PREHASH_ImageID, _PREHASH_Codec, codec);
	msg->getU16Fast(_PREHASH_ImageID, _PREHASH_Packets, packets);
	msg->getU32Fast(_PREHASH_ImageID, _PREHASH_Size, totalbytes);
	
	S32 data_size = msg->getSizeFast(_PREHASH_ImageData, _PREHASH_Data); 
	if (!data_size)
	{
		return;
	}
	if (data_size < 0)
	{
		// msg->getSizeFast() is probably trying to tell us there
		// was an error.
		llerrs << "image header chunk size was negative: "
		<< data_size << llendl;
		return;
	}
	
	// this buffer gets saved off in the packet list
	U8 *data = new U8[data_size];
	msg->getBinaryDataFast(_PREHASH_ImageData, _PREHASH_Data, data, data_size);
	
	LLViewerImage *image = gImageList.getImage(id);
	if (!image)
	{
		delete [] data;
		return;
	}
	image->mLastPacketTimer.reset();
	bool res = LLAppViewer::getTextureFetch()->receiveImageHeader(msg->getSender(), id, codec, packets, totalbytes, data_size, data);
	if (!res)
	{
		delete[] data;
	}
}

// static
void LLViewerImageList::receiveImagePacket(LLMessageSystem *msg, void **user_data)
{
	LLMemType mt1(LLMemType::MTYPE_APPFMTIMAGE);
	LLFastTimer t(LLFastTimer::FTM_PROCESS_IMAGES);
	
	// Receives image packet, copy into image object,
	// checks if all packets received, decompresses if so. 
	
	LLUUID id;
	U16 packet_num;
	
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
	S32 data_size = msg->getSizeFast(_PREHASH_ImageData, _PREHASH_Data); 
	
	if (!data_size)
	{
		return;
	}
	if (data_size < 0)
	{
		// msg->getSizeFast() is probably trying to tell us there
		// was an error.
		llerrs << "image data chunk size was negative: "
		<< data_size << llendl;
		return;
	}
	if (data_size > MTUBYTES)
	{
		llerrs << "image data chunk too large: " << data_size << " bytes" << llendl;
		return;
	}
	U8 *data = new U8[data_size];
	msg->getBinaryDataFast(_PREHASH_ImageData, _PREHASH_Data, data, data_size);
	
	LLViewerImage *image = gImageList.getImage(id);
	if (!image)
	{
		delete [] data;
		return;
	}
	image->mLastPacketTimer.reset();
	bool res = LLAppViewer::getTextureFetch()->receiveImagePacket(msg->getSender(), id, packet_num, data_size, data);
	if (!res)
	{
		delete[] data;
	}
}


// We've been that the asset server does not contain the requested image id.
// static
void LLViewerImageList::processImageNotInDatabase(LLMessageSystem *msg,void **user_data)
{
	LLFastTimer t(LLFastTimer::FTM_PROCESS_IMAGES);
	LLUUID image_id;
	msg->getUUIDFast(_PREHASH_ImageID, _PREHASH_ID, image_id);
	
	LLViewerImage* image = gImageList.hasImage( image_id );
	if( image )
	{
		image->setIsMissingAsset();
	}
}

///////////////////////////////////////////////////////////////////////////////

//static
const U32 SIXTEEN_MEG = 0x1000000;
S32 LLViewerImageList::calcMaxTextureRAM()
{
	// Decide the maximum amount of RAM we should allow the user to allocate to texture cache
	LLMemoryInfo memory_info;
	U32 available_memory = memory_info.getPhysicalMemoryClamped();
	
	clamp_rescale((F32)available_memory,
				  (F32)(SIXTEEN_MEG * 16),
				  (F32)U32_MAX,
				  (F32)(SIXTEEN_MEG * 4),
				  (F32)(U32_MAX >> 1));
	return available_memory;
}

///////////////////////////////////////////////////////////////////////////////

// explicitly cleanup resources, as this is a singleton class with process
// lifetime so ability to perform std::map operations in destructor is not
// guaranteed.
void LLUIImageList::cleanUp()
{
	mUIImages.clear();
}

LLUIImagePtr LLUIImageList::getUIImageByID(const LLUUID& image_id)
{
	// use id as image name
	std::string image_name = image_id.asString();

	// look for existing image
	uuid_ui_image_map_t::iterator found_it = mUIImages.find(image_name);
	if (found_it != mUIImages.end())
	{
		return found_it->second;
	}

	return loadUIImageByID(image_id);
}

LLUIImagePtr LLUIImageList::getUIImage(const std::string& image_name)
{
	// look for existing image
	uuid_ui_image_map_t::iterator found_it = mUIImages.find(image_name);
	if (found_it != mUIImages.end())
	{
		return found_it->second;
	}

	return loadUIImageByName(image_name, image_name);
}

LLUIImagePtr LLUIImageList::loadUIImageByName(const std::string& name, const std::string& filename, BOOL use_mips, const LLRect& scale_rect)
{
	LLViewerImage* imagep = gImageList.getImageFromFile(filename, MIPMAP_NO, IMMEDIATE_YES);
	return loadUIImage(imagep, name, use_mips, scale_rect);
}

LLUIImagePtr LLUIImageList::loadUIImageByID(const LLUUID& id, BOOL use_mips, const LLRect& scale_rect)
{
	LLViewerImage* imagep = gImageList.getImage(id, MIPMAP_NO, IMMEDIATE_YES);
	return loadUIImage(imagep, id.asString(), use_mips, scale_rect);
}

LLUIImagePtr LLUIImageList::loadUIImage(LLViewerImage* imagep, const std::string& name, BOOL use_mips, const LLRect& scale_rect)
{
	if (!imagep) return NULL;

	imagep->setAddressMode(LLTexUnit::TAM_CLAMP);

	//all UI images are non-deletable
	imagep->setNoDelete() ;

	LLUIImagePtr new_imagep = new LLUIImage(name, imagep);
	mUIImages.insert(std::make_pair(name, new_imagep));

	LLUIImageLoadData* datap = new LLUIImageLoadData;
	datap->mImageName = name;
	datap->mImageScaleRegion = scale_rect;

	imagep->setLoadedCallback(onUIImageLoaded, 0, FALSE, FALSE, datap);

	return new_imagep;
}

LLUIImagePtr LLUIImageList::preloadUIImage(const std::string& name, const std::string& filename, BOOL use_mips, const LLRect& scale_rect)
{
	// look for existing image
	uuid_ui_image_map_t::iterator found_it = mUIImages.find(name);
	if (found_it != mUIImages.end())
	{
		// image already loaded!
		llerrs << "UI Image " << name << " already loaded." << llendl;
	}

	return loadUIImageByName(name, filename, use_mips, scale_rect);
}

//static 
void LLUIImageList::onUIImageLoaded( BOOL success, LLViewerImage *src_vi, LLImageRaw* src, LLImageRaw* src_aux, S32 discard_level, BOOL final, void* user_data )
{
	if(!success || !user_data) 
	{
		return;
	}

	LLUIImageLoadData* image_datap = (LLUIImageLoadData*)user_data;
	std::string ui_image_name = image_datap->mImageName;
	LLRect scale_rect = image_datap->mImageScaleRegion;
	if (final)
	{
		delete image_datap;
	}

	LLUIImageList* instance = getInstance();

	uuid_ui_image_map_t::iterator found_it = instance->mUIImages.find(ui_image_name);
	if (found_it != instance->mUIImages.end())
	{
		LLUIImagePtr imagep = found_it->second;

		// for images grabbed from local files, apply clipping rectangle to restore original dimensions
		// from power-of-2 gl image
		if (success && imagep.notNull() && src_vi && (src_vi->mUrl.compare(0, 7, "file://")==0))
		{
			F32 clip_x = (F32)src_vi->getOriginalWidth() / (F32)src_vi->getWidth(0);
			F32 clip_y = (F32)src_vi->getOriginalHeight() / (F32)src_vi->getHeight(0);
			imagep->setClipRegion(LLRectf(0.f, clip_y, clip_x, 0.f));
			if (scale_rect != LLRect::null)
			{
				imagep->setScaleRegion(
					LLRectf(llclamp((F32)scale_rect.mLeft / (F32)imagep->getWidth(), 0.f, 1.f),
						llclamp((F32)scale_rect.mTop / (F32)imagep->getHeight(), 0.f, 1.f),
						llclamp((F32)scale_rect.mRight / (F32)imagep->getWidth(), 0.f, 1.f),
						llclamp((F32)scale_rect.mBottom / (F32)imagep->getHeight(), 0.f, 1.f)));
			}
		}
	}
}

bool LLUIImageList::initFromFile()
{
	// construct path to canonical textures.xml in default skin dir
	std::string base_file_path = gDirUtilp->getExpandedFilename(LL_PATH_SKINS, "default", "textures", "textures.xml");

	LLXMLNodePtr root;

	if (!LLXMLNode::parseFile(base_file_path, root, NULL))
	{
		llwarns << "Unable to parse UI image list file " << base_file_path << llendl;
		return false;
	}

	if (!root->hasAttribute("version"))
	{
		llwarns << "No valid version number in UI image list file " << base_file_path << llendl;
		return false;
	}

	std::vector<std::string> paths;
	// path to current selected skin
	paths.push_back(gDirUtilp->getSkinDir() 
			+ gDirUtilp->getDirDelimiter() 
			+ "textures"
			+ gDirUtilp->getDirDelimiter()
			+ "textures.xml");
	// path to user overrides on current skin
	paths.push_back(gDirUtilp->getUserSkinDir() 
			+ gDirUtilp->getDirDelimiter() 
			+ "textures"
			+ gDirUtilp->getDirDelimiter()
			+ "textures.xml");

	// apply skinned xml files incrementally
	for(std::vector<std::string>::iterator path_it = paths.begin();
		path_it != paths.end();
		++path_it)
	{
		// don't reapply base file to itself
		if (!path_it->empty() && (*path_it) != base_file_path)
		{
			LLXMLNodePtr update_root;
			if (LLXMLNode::parseFile(*path_it, update_root, NULL))
			{
				LLXMLNode::updateNode(root, update_root);
			}
		}
	}

	enum
	{
		PASS_DECODE_NOW,
		PASS_DECODE_LATER,
		NUM_PASSES
	};

	for (S32 pass = PASS_DECODE_NOW; pass < NUM_PASSES; pass++)
	{
		LLXMLNodePtr child_nodep = root->getFirstChild();
		while(child_nodep.notNull())
		{
			std::string image_name;
			child_nodep->getAttributeString("name", image_name);

			std::string file_name = image_name;
			LLRect scale_rect;
			BOOL use_mip_maps = FALSE;

			BOOL preload = FALSE;
			child_nodep->getAttributeBOOL("preload", preload);

			// load high priority textures on first pass (to kick off decode)
			if (preload)
			{
				if (pass == PASS_DECODE_LATER) 
				{
					child_nodep = child_nodep->getNextSibling();
					continue;
				}
			}
			else
			{
				if (pass == PASS_DECODE_NOW)
				{
					child_nodep = child_nodep->getNextSibling();
					continue;
				}
			}

			child_nodep->getAttributeString("file_name", file_name);
			child_nodep->getAttributeBOOL("use_mips", use_mip_maps);

			child_nodep->getAttributeS32("scale_left", scale_rect.mLeft);
			child_nodep->getAttributeS32("scale_right", scale_rect.mRight);
			child_nodep->getAttributeS32("scale_bottom", scale_rect.mBottom);
			child_nodep->getAttributeS32("scale_top", scale_rect.mTop);
			
			preloadUIImage(image_name, file_name, use_mip_maps, scale_rect);
			
			child_nodep = child_nodep->getNextSibling();
		}

		if (pass == PASS_DECODE_NOW && !gSavedSettings.getBOOL("NoPreload"))
		{
			gImageList.decodeAllImages(10.f); // decode preloaded images
		}
	}
	return true;
}


