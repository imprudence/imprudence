/** 
 * @file llviewerimagelist.cpp
 * @brief Object for managing the list of images within a region
 *
 * Copyright (c) 2000-2007, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include <sys/stat.h>

#include "llviewerimagelist.h"
#include "imageids.h"
#include "llgl.h" // fot gathering stats from GL
#include "llimagegl.h"
#include "llimagebmp.h"
#include "llimagej2c.h"
#include "llimagetga.h"
#include "llimagejpeg.h"
#include "llmediaengine.h"

#include "llsdserialize.h"
#include "llsys.h"
#include "llvfs.h"
#include "llvfile.h"
#include "llvfsthread.h"
#include "message.h"

#include "llagent.h"
#include "lltexturecache.h"
#include "lltexturefetch.h"
#include "lltexturetable.h" // For looking up names from uuid's.
#include "llviewercontrol.h"
#include "llviewerimage.h"
#include "llviewerregion.h"
#include "pipeline.h"
#include "viewer.h"

////////////////////////////////////////////////////////////////////////////

void (*LLViewerImageList::sUUIDCallback)(void **, const LLUUID&) = NULL;

U32 LLViewerImageList::sTextureBits = 0;
U32 LLViewerImageList::sTexturePackets = 0;

const S32 IMAGES_PER_REQUEST = 42;
const S32 IMAGES_MIN_UPDATES = 4;  // Always update the highest N images each frame
const S32 IMAGES_MAX_PACKET_UPDATES = 1; // Only send N packets of IMAGES_PER_REQUEST in a frame
const F32 RESEND_IMAGE_REQUEST_TIME = 15.f; // seconds

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
	: LLImageProviderInterface(),
	  mForceResetTextureStats(FALSE),
	  mUpdateStats(FALSE),
	  mMaxResidentTexMem(0),
	  mVideoMemorySetting(0),
	  mMovieImageHasMips(FALSE)	
{
}

void LLViewerImageList::init()
{
	sNumImages = 0;
	mMaxResidentTexMem = 0;
	mVideoMemorySetting = 0;

	if (gNoRender)
	{
		// Don't initialize GL stuff if we're not rendering.
		return;
	}

	mUpdateStats = TRUE;

	// Update how much texture RAM we're allowed to use.
	updateMaxResidentTexMem();

	mMovieImageHasMips = FALSE;

	doPreloadImages();

	decodeAllImages(5.f); // decode preloaded images
}

void LLViewerImageList::doPreloadImages()
{
	llinfos << "Preloading images..." << llendl;

	// Set the "missing asset" image
	LLViewerImage::sMissingAssetImagep = preloadImage("missing_asset.tga" , LLUUID::null, TRUE);

	// Set the "white" image
	LLViewerImage::sWhiteImagep = preloadImage("white.tga", LLUUID::null, TRUE);;

	// Speeds up startup by 4-5 seconds. JC
	if (!gPreloadImages) return;

	// Preload some images
	preloadImage("button_anim_pause.tga", LLUUID::null, FALSE);
	preloadImage("button_anim_pause_selected.tga", LLUUID::null, FALSE);
	preloadImage("button_anim_play.tga", LLUUID::null, FALSE);
	preloadImage("button_anim_play_selected.tga", LLUUID::null, FALSE);
	preloadImage("button_anim_stop.tga", LLUUID::null, FALSE);
	preloadImage("button_anim_stop_selected.tga", LLUUID::null, FALSE);
	preloadImage("button_disabled_32x128.tga", LLUUID::null, FALSE);
	preloadImage("button_enabled_32x128.tga", LLUUID::null, FALSE);
	preloadImage("button_enabled_selected_32x128.tga", LLUUID::null, FALSE);
	preloadImage("checkbox_disabled_false.tga", LLUUID::null, FALSE);
	preloadImage("checkbox_disabled_true.tga", LLUUID::null, FALSE);
	preloadImage("checkbox_enabled_false.tga", LLUUID::null, FALSE);
	preloadImage("checkbox_enabled_true.tga", LLUUID::null, FALSE);
	preloadImage("close_in_blue.tga", LLUUID::null, FALSE);
	preloadImage("combobox_arrow.tga", LLUUID::null, FALSE);
	preloadImage("crosshairs.tga", LLUUID::null, FALSE);
	preloadImage("direction_arrow.tga", LLUUID::null, FALSE);
	preloadImage("eyes.tga", LLUUID::null, TRUE);
	preloadImage("foot_shadow.tga", LLUUID::null, TRUE);
	preloadImage("hair.tga", LLUUID::null, TRUE);
	preloadImage("icon_for_sale.tga", LLUUID::null, FALSE);
	preloadImage("icon_popular.tga", LLUUID::null, FALSE);
	preloadImage("icon_top_pick.tga", LLUUID::null, FALSE);
	preloadImage("img_shot.tga", IMG_SHOT, TRUE);
	preloadImage("img_smoke_poof.tga", IMG_SMOKE_POOF, TRUE);
	preloadImage("inv_folder_animation.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_bodypart.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_callingcard.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_clothing.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_gesture.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_landmark.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_lostandfound.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_notecard.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_object.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_plain_closed.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_script.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_snapshot.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_sound.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_texture.tga", LLUUID::null, FALSE);
	preloadImage("inv_folder_trash.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_animation.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_bodypart.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_callingcard_offline.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_callingcard_online.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_eyes.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_gesture.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_gloves.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_hair.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_jacket.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_landmark.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_landmark_visited.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_notecard.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_object.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_object_multi.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_pants.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_script.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_shape.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_shirt.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_shoes.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_skirt.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_snapshot.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_socks.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_sound.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_texture.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_underpants.tga", LLUUID::null, FALSE);
	preloadImage("inv_item_undershirt.tga", LLUUID::null, FALSE);
	preloadImage("legend.tga", LLUUID::null, FALSE);
	preloadImage("map_avatar_16.tga", LLUUID::null, FALSE);
	preloadImage("map_avatar_8.tga", LLUUID::null, FALSE);
	preloadImage("map_avatar_you_8.tga", LLUUID::null, FALSE);
	preloadImage("map_event.tga", LLUUID::null, FALSE);
	preloadImage("map_event_mature.tga", LLUUID::null, FALSE);
	preloadImage("map_home.tga", LLUUID::null, FALSE);
	preloadImage("map_infohub.tga", LLUUID::null, FALSE);
	preloadImage("map_telehub.tga", LLUUID::null, FALSE);
	preloadImage("map_track_16.tga", LLUUID::null, FALSE);
	preloadImage("minimize.tga", LLUUID::null, FALSE);
	preloadImage("minimize_pressed.tga", LLUUID::null, FALSE);
	preloadImage("noentrylines.tga", LLUUID::null, TRUE);
	preloadImage("noentrypasslines.tga", LLUUID::null, TRUE);
	preloadImage("notify_tip_icon.tga", LLUUID::null, FALSE);
	preloadImage("object_cone.tga", LLUUID::null, FALSE);
	preloadImage("object_cone_active.tga", LLUUID::null, FALSE);
	preloadImage("object_cube.tga", LLUUID::null, FALSE);
	preloadImage("object_cube_active.tga", LLUUID::null, FALSE);
	preloadImage("object_cylinder.tga", LLUUID::null, FALSE);
	preloadImage("object_cylinder_active.tga", LLUUID::null, FALSE);
	preloadImage("object_grass.tga", LLUUID::null, FALSE);
	preloadImage("object_grass_active.tga", LLUUID::null, FALSE);
	preloadImage("object_hemi_cone.tga", LLUUID::null, FALSE);
	preloadImage("object_hemi_cone_active.tga", LLUUID::null, FALSE);
	preloadImage("object_hemi_cylinder.tga", LLUUID::null, FALSE);
	preloadImage("object_hemi_cylinder_active.tga", LLUUID::null, FALSE);
	preloadImage("object_hemi_sphere.tga", LLUUID::null, FALSE);
	preloadImage("object_hemi_sphere_active.tga", LLUUID::null, FALSE);
	preloadImage("object_prism.tga", LLUUID::null, FALSE);
	preloadImage("object_prism_active.tga", LLUUID::null, FALSE);
	preloadImage("object_pyramid.tga", LLUUID::null, FALSE);
	preloadImage("object_pyramid_active.tga", LLUUID::null, FALSE);
	preloadImage("object_ring.tga", LLUUID::null, FALSE);
	preloadImage("object_ring_active.tga", LLUUID::null, FALSE);
	preloadImage("object_sphere.tga", LLUUID::null, FALSE);
	preloadImage("object_sphere_active.tga", LLUUID::null, FALSE);
	preloadImage("object_tetrahedron.tga", LLUUID::null, FALSE);
	preloadImage("object_tetrahedron_active.tga", LLUUID::null, FALSE);
	preloadImage("object_torus.tga", LLUUID::null, FALSE);
	preloadImage("object_torus_active.tga", LLUUID::null, FALSE);
	preloadImage("object_tree.tga", LLUUID::null, FALSE);
	preloadImage("object_tree_active.tga", LLUUID::null, FALSE);
	preloadImage("object_tube.tga", LLUUID::null, FALSE);
	preloadImage("object_tube_active.tga", LLUUID::null, FALSE);
	preloadImage("pixiesmall.tga", LLUUID::null, TRUE);
	preloadImage("radio_active_false.tga", LLUUID::null, FALSE);
	preloadImage("radio_active_true.tga", LLUUID::null, FALSE);
	preloadImage("radio_inactive_false.tga", LLUUID::null, FALSE);
	preloadImage("radio_inactive_true.tga", LLUUID::null, FALSE);
	preloadImage("resize_handle_bottom_right_blue.tga", LLUUID::null, FALSE);
	preloadImage("rounded_square.tga", LLUUID::null, FALSE);
	preloadImage("rounded_square_soft.tga", LLUUID::null, FALSE);
	preloadImage("script_error.tga", LLUUID::null, TRUE);
	preloadImage("scrollbutton_down_in_blue.tga", LLUUID::null, FALSE);
	preloadImage("scrollbutton_down_out_blue.tga", LLUUID::null, FALSE);
	preloadImage("scrollbutton_left_in_blue.tga", LLUUID::null, FALSE);
	preloadImage("scrollbutton_left_out_blue.tga", LLUUID::null, FALSE);
	preloadImage("scrollbutton_right_in_blue.tga", LLUUID::null, FALSE);
	preloadImage("scrollbutton_right_out_blue.tga", LLUUID::null, FALSE);
	preloadImage("scrollbutton_up_in_blue.tga", LLUUID::null, FALSE);
	preloadImage("scrollbutton_up_out_blue.tga", LLUUID::null, FALSE);
	preloadImage("silhouette.tga", LLUUID::null, TRUE);
	preloadImage("spin_down_in_blue.tga", LLUUID::null, FALSE);
	preloadImage("spin_down_out_blue.tga", LLUUID::null, FALSE);
	preloadImage("spin_up_in_blue.tga", LLUUID::null, FALSE);
	preloadImage("spin_up_out_blue.tga", LLUUID::null, FALSE);
	preloadImage("square_btn_32x128.tga", LLUUID::null, FALSE);
	preloadImage("square_btn_selected_32x128.tga", LLUUID::null, FALSE);
	preloadImage("startup_logo.tga", LLUUID::null, FALSE);
	preloadImage("status_build.tga", LLUUID::null, FALSE);
	preloadImage("status_buy_currency.tga", LLUUID::null, FALSE);
	preloadImage("status_buy_currency_pressed.tga", LLUUID::null, FALSE);
	preloadImage("status_buy_land.tga", LLUUID::null, FALSE);
	preloadImage("status_buy_land_pressed.tga", LLUUID::null, FALSE);
	preloadImage("status_fly.tga", LLUUID::null, FALSE);
	preloadImage("status_health.tga", LLUUID::null, FALSE);
	preloadImage("status_scripts.tga", LLUUID::null, FALSE);
	preloadImage("tab_bottom_blue.tga", LLUUID::null, FALSE);
	preloadImage("tab_bottom_selected_blue.tga", LLUUID::null, FALSE);
	preloadImage("tab_left.tga", LLUUID::null, FALSE);
	preloadImage("tab_left_selected.tga", LLUUID::null, FALSE);
	preloadImage("tab_top_blue.tga", LLUUID::null, FALSE);
	preloadImage("tab_top_selected_blue.tga", LLUUID::null, FALSE);
	preloadImage("tool_dozer.tga", LLUUID::null, FALSE);
	preloadImage("tool_dozer_active.tga", LLUUID::null, FALSE);
	preloadImage("tool_zoom.tga", LLUUID::null, FALSE);
	preloadImage("tool_zoom_active.tga", LLUUID::null, FALSE);
	preloadImage("white.tga", LLUUID::null, TRUE);
}

static std::string get_texture_list_name()
{
	BOOL login_last = gSavedSettings.getBOOL("LoginLastLocation");
	return std::string("texture_list_") + (login_last?"last":"home") + ".xml";
}

void LLViewerImageList::doPrefetchImages()
{
	if (gPurgeCache)
	{
		// cache was purged, no point
		return;
	}
	
	// Pre-fetch textures from last logout
	LLSD imagelist;
	std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, get_texture_list_name());
	llifstream file;
	file.open(filename.c_str());
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

	
}

///////////////////////////////////////////////////////////////////////////////

LLViewerImageList::~LLViewerImageList()
{
	llassert(mIRCallbackData.empty());
}

void LLViewerImageList::shutdown()
{
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
		file.open(filename.c_str());
		LLSDSerialize::toPrettyXML(imagelist, file);
	}
	
	//
	// Clean up "loaded" callbacks.
	//
	mCallbackList.clear();
	mIRCallbackData.clear();
	
	// Clean up preloaded images
	mPreloadedImages.clear();
	
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

LLImageGL* LLViewerImageList::getUIImageByID(const LLUUID& image_id, BOOL clamped)
{
	LLViewerImage* imagep = getImage(image_id, MIPMAP_FALSE, TRUE);
	// force a high resolution decode for all UI images (pulled this from LLTextEditor)
	// this might not make any difference
	imagep->setBoostLevel(LLViewerImage::BOOST_UI);
	LLViewerImage::bindTexture(imagep);
	imagep->setClamp(clamped, clamped);
	imagep->unbindTexture(0, GL_TEXTURE_2D);

	return (LLImageGL*)imagep;
}

///////////////////////////////////////////////////////////////////////////////

LLViewerImage* LLViewerImageList::preloadImage(const LLString& filename, const LLUUID &image_set_id, BOOL use_mips)
{
	LLViewerImage* image = getImage(filename, image_set_id, use_mips, TRUE);
	image->dontDiscard();
	mPreloadedImages.push_back(image);
	return image;
}

///////////////////////////////////////////////////////////////////////////////

LLViewerImage* LLViewerImageList::getImage(const LLString& filename,
										   const LLUUID &image_set_id,
										   BOOL usemipmaps,
										   BOOL level_immediate)
{
	return getImageFromFile(filename, image_set_id, usemipmaps, level_immediate, 0, 0);
}

LLViewerImage* LLViewerImageList::getImageFromFile(const LLString& filename,
												   const LLUUID &image_set_id,
												   BOOL usemipmaps,
												   BOOL level_immediate,
												   LLGLint internal_format,
												   LLGLenum primary_format)
{
	if (gNoRender)
	{
		// Never mind that this ignores image_set_id;
		// getImage() will handle that later.
		return getImage(IMG_DEFAULT, TRUE, TRUE);
	}
	
	// Try to load an image from the skins directory.
	// Fall back to loading from the VFS if not found.

	// First verify that the image exists in gViewerArt
	LLUUID image_id = LLUUID( gViewerArt.getString(filename.c_str()) );
	if (image_id.isNull())
	{
		llwarns << "Unable to find image " << filename << " in gViewerArt" << llendl;
		if (image_set_id.notNull())
		{
			// We *know* that missing_asset.tga exists,
			// but for paranoia's sake and to avoid infinite recursion, check anyway
			image_id = LLUUID(gViewerArt.getString("missing_asset.tga"));
			if (image_id.isNull())
			{
				llerrs << "Missing missing_asset.tga!" << llendl;
			}
			return getImageFromFile(LLString("missing_asset.tga"), image_set_id,
									usemipmaps, level_immediate,
									internal_format, primary_format);
		}
		else
		{
			return (getImage(IMG_DEFAULT, TRUE, TRUE));
		}
	}

	// Now that we have verified that filename exists, load it and assign it to
	// the filename's UUID, or image_set_id if non null.
	if (image_set_id.notNull())
	{
		image_id = image_set_id;
	}
	
	// Load the image
	LLViewerImage* imagep = getImageFromUUID(image_id, usemipmaps, level_immediate, 
											 internal_format, primary_format, LLHost());

	return imagep;
}

LLViewerImage* LLViewerImageList::getImage(const LLUUID &image_id,
										   BOOL usemipmaps,
										   BOOL level_immediate)
{
	return getImageFromUUID(image_id, usemipmaps, level_immediate, 0, 0, LLHost());
}

LLViewerImage* LLViewerImageList::getImageFromUUID(const LLUUID &image_id,
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
		imagep = new LLViewerImage(image_id, usemipmaps);
		// Might want to request from host other than where the agent is. JC
		imagep->setTargetHost(request_from_host);

		if (internal_format && primary_format)
		{
			imagep->setExplicitFormat(internal_format, primary_format);
		}

		addImage(imagep);

		if (level_immediate)
		{
			imagep->dontDiscard();
			imagep->setBoostLevel(LLViewerImage::BOOST_UI);
		}
	}
	
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
		llerrs << "Image with ID " << image_id << " already in list" << llendl;
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

void LLViewerImageList::updateMovieImage(const LLUUID& uuid, BOOL active)
{
	// IF the media image hasn't changed, do nothing
	if (mMovieImageUUID == uuid)
	{
		return;
	}
	// If we have changed media uuid, restore the old one
	if (!mMovieImageUUID.isNull())
	{
		LLViewerImage* oldImage = getImage( mMovieImageUUID );
		if (oldImage)
		{
			oldImage->reinit(mMovieImageHasMips);
			oldImage->mIsMediaTexture = FALSE;
		}
		mMovieImageUUID.setNull();
	}
	// If the movie is playing, set the new media image
	if (active && !uuid.isNull())
	{
		LLViewerImage* viewerImage = getImage( uuid );
		if( viewerImage )
		{
			mMovieImageUUID = uuid;
			// Can't use mipmaps for movies because they don't update the full image
			mMovieImageHasMips = viewerImage->getUseMipMaps();
			viewerImage->reinit(FALSE);
			viewerImage->mIsMediaTexture = TRUE;
		}
	}
}

////////////////////////////////////////////////////////////////////////////

void LLViewerImageList::dirtyImage(LLViewerImage *image)
{
	mDirtyTextureList.insert(image);
}

////////////////////////////////////////////////////////////////////////////

void LLViewerImageList::updateImages(F32 max_time)
{
	sNumImagesStat.addValue(sNumImages);
	sNumRawImagesStat.addValue(LLImageRaw::sRawImageCount);
	sGLTexMemStat.addValue(LLImageGL::sGlobalTextureMemory/(1024.f*1024.f));
	sGLBoundMemStat.addValue(LLImageGL::sBoundTextureMemory/(1024.f*1024.f));
	sRawMemStat.addValue(LLImageRaw::sGlobalRawMemory/(1024.f*1024.f));
	sFormattedMemStat.addValue(LLImageFormatted::sGlobalFormattedMemory/(1024.f*1024.f));

	updateImagesDecodePriorities();
	max_time -= updateImagesFetchTextures(max_time);
	max_time = llmax(max_time, 0.001f);
	max_time -= updateImagesCreateTextures(max_time);
	max_time = llmax(max_time, 0.001f);

	if (!mDirtyTextureList.empty())
	{
		LLFastTimer t(LLFastTimer::FTM_IMAGE_MARK_DIRTY);
		gPipeline.dirtyPoolObjectTextures(mDirtyTextureList);
		mDirtyTextureList.clear();
	}

	for (image_list_t::iterator iter = mCallbackList.begin();
		 iter != mCallbackList.end(); )
	{
		LLViewerImage* image = *iter++;
		// Do stuff to handle callbacks, update priorities, etc.
		bool res = image->doLoadedCallbacks();
		if (res)
		{
			break; // only actually do one callback per frame
		}
	}
	
	updateImagesMediaStreams();
	updateImagesUpdateStats();
}

void LLViewerImageList::updateImagesDecodePriorities()
{
	// Update the decode priority for N images each frame
	{
		const size_t max_update_count = 256;
		S32 update_counter = llmin(max_update_count, mUUIDMap.size()/10);
		uuid_map_t::iterator iter = mUUIDMap.upper_bound(mLastUpdateUUID);
		while(update_counter > 0)
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
			const F32 LAZY_FLUSH_TIMEOUT = 30.f;
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
					continue;
				}
			}
			else
			{
				imagep->mLastReferencedTimer.reset();
			}
			
			imagep->processTextureStats();
			F32 old_priority = imagep->getDecodePriority();
			F32 decode_priority = imagep->calcDecodePriority();
			// Ignore < 20% difference
			if ((decode_priority < old_priority * .8f || decode_priority > old_priority * 1.25f))
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

F32 LLViewerImageList::updateImagesFetchTextures(F32 max_time)
{
	LLTimer image_op_timer;
	
	// Update the decode priority for N images each frame
	// Make a list with 32 high priority entries + 256 cycled entries
	const size_t max_priority_count = 32;
	const size_t max_update_count = 256;

	// 32 high priority entries
	std::set<LLViewerImage*> entries;
	size_t update_counter = llmin(max_priority_count, mImageList.size());
	image_priority_list_t::iterator iter1 = mImageList.begin();
	while(update_counter > 0)
	{
		entries.insert(*iter1);
		++iter1;
		update_counter--;
	}
	
	// 256 cycled entries
	update_counter = llmin(max_update_count, mUUIDMap.size());
	uuid_map_t::iterator iter2 = mUUIDMap.upper_bound(mLastFetchUUID);
	while(update_counter > 0)
	{
		if (iter2 == mUUIDMap.end())
		{
			iter2 = mUUIDMap.begin();
		}
		mLastFetchUUID = iter2->first;
		entries.insert(iter2->second);
		++iter2;
		update_counter--;
	}

	S32 min_count = max_priority_count + max_update_count/4;
	for (std::set<LLViewerImage*>::iterator iter3 = entries.begin();
		 iter3 != entries.end(); )
	{
		LLPointer<LLViewerImage> imagep = *iter3++;

		imagep->updateFetch();
		if (min_count <= 0 && image_op_timer.getElapsedTimeF32() > max_time)
		{
			break;
		}
		min_count--;
	}
	return image_op_timer.getElapsedTimeF32();
}

void LLViewerImageList::updateImagesMediaStreams()
{
	if (gNoRender || gGLManager.mIsDisabled) return;
	
		// update media stream if required
		LLMediaEngine* media_engine = LLMediaEngine::getInstance();
		if (media_engine)
		{
			if ( media_engine->update() )
			{
				LLUUID media_uuid = media_engine->getImageUUID();
				updateMovieImage(media_uuid, TRUE);
				if (!media_uuid.isNull())
				{
					LLViewerImage* viewerImage = getImage( media_uuid );
					if( viewerImage )
					{
						LLMediaBase* renderer = media_engine->getMediaRenderer();
						if ((renderer->getTextureWidth() != viewerImage->getWidth()) ||
							(renderer->getTextureHeight() != viewerImage->getHeight()) ||
							(renderer->getTextureDepth() != viewerImage->getComponents()) ||
							(viewerImage->getHasGLTexture() == FALSE))
						{
							// destroy existing GL image
							viewerImage->destroyGLTexture();
					
							// set new size
							viewerImage->setSize( renderer->getTextureWidth(),
												  renderer->getTextureHeight(),
												  renderer->getTextureDepth() );

							LLPointer<LLImageRaw> raw = new LLImageRaw(renderer->getTextureWidth(),
																	   renderer->getTextureHeight(),
																	   renderer->getTextureDepth());
							raw->clear(0x7f,0x7f,0x7f,0xff);
							viewerImage->createGLTexture(0, raw);
						}

						// Set the explicit format the instance wants
						viewerImage->setExplicitFormat(renderer->getTextureFormatInternal(), 
													   renderer->getTextureFormatPrimary(), 
													   renderer->getTextureFormatType(),
													   renderer->getTextureFormatSwapBytes());
						// This should be redundant, but just in case:
						viewerImage->setUseMipMaps(FALSE);

						LLImageRaw* rawImage = media_engine->getImageRaw();
						if ( rawImage )
						{
							viewerImage->setSubImage(rawImage, 0, 0,
													 renderer->getMediaWidth(),
													 renderer->getMediaHeight());
						}
					}
					else
					{
						llwarns << "MediaEngine update unable to get viewer image for GL texture" << llendl;
					}
				}
			}
			else
			{
				LLUUID media_uuid = media_engine->getImageUUID();
				updateMovieImage(media_uuid, FALSE);
			}
		}
}

void LLViewerImageList::updateImagesUpdateStats()
{
	if (mUpdateStats)
	{
		for (image_priority_list_t::iterator iter = mImageList.begin();
			 iter != mImageList.end(); )
		{
			LLViewerImage* imagep = *iter++;
			imagep->resetTextureStats(mForceResetTextureStats);
		}
		mUpdateStats = FALSE;
		mForceResetTextureStats = FALSE;
	}
}

void LLViewerImageList::decodeAllImages(F32 max_time)
{
	LLTimer timer;
	if(!gNoRender)
	{
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
		while (1)
		{
			gTextureCache->update(1); // unpauses the texture cache thread
			gImageDecodeThread->update(1); // unpauses the image thread
			S32 fetch_pending = gTextureFetch->update(1); // unpauses the texture fetch thread
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
		max_time = llmax(max_time, .01f);
		updateImagesCreateTextures(max_time);
	}
	if (timer.getElapsedTimeF32() > .5f) // seconds
	{
		llinfos << "decodeAllImages() took " << timer.getElapsedTimeF32() << " seconds. " << llendl;
	}
}


BOOL LLViewerImageList::createUploadFile(const LLString& filename,
										 const LLString& out_filename,
										 const U8 codec)
{
	// First, load the image.
	LLPointer<LLImageRaw> raw_image = new LLImageRaw;

	switch (codec)
	{
	  case IMG_CODEC_BMP:
	  {
		  LLPointer<LLImageBMP> bmp_image = new LLImageBMP;

		  if (!bmp_image->load(filename))
		  {
			  return FALSE;
		  }

		  if (!bmp_image->decode(raw_image))
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

		  if (!jpeg_image->decode(raw_image))
		  {
			  return FALSE;
		  }
	  }
	  break;
	  default:
		return FALSE;
	}

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
	compressedImage->encode(raw_image);

	return compressedImage;
}

//static
S32 LLViewerImageList::getMaxVideoRamSetting(S32 max)
{
	const U32 vram_settings[] = { 16, 32, 64, 128, 256, 512 };
	const S32 num_vram_settings = sizeof(vram_settings) / sizeof(vram_settings[0]);

	U32 max_vram;
	if (gGLManager.mVRAM != 0)
	{
		max_vram = (llmax(gGLManager.mVRAM,16)) << 20;
	}
	else
	{
		if (max == -2) // max recommended setting
		{
			max_vram = 128 << 20;
		}
		else
		{
			max_vram = 512 << 20;
		}
		llwarns << "VRAM amount not detected, defaulting to " << max_vram/(double)(1<<20) << " MB" << llendl;
	}
	U32 system_ram = gSysMemory.getPhysicalMemory();
	//llinfos << "*** DETECTED " << system_ram/(double)(1<<20) << " MB of system memory." << llendl; // TomY TESTING DNCI
	if (max == -2)
	{
		max_vram = llmin(max_vram, (U32)(system_ram/2)); // max recommended setting
	}
	else
	{
		max_vram = llmin(max_vram, (U32)((F32)system_ram/1.5f));
	}

	S32 idx;
	for (idx=0; idx < num_vram_settings; idx++)
	{
		if (idx == max)
			break;
		if ((vram_settings[idx] << 20) > max_vram)
		{
			idx--;
			break;
		}
	}

	if( idx == num_vram_settings )
	{
		idx = num_vram_settings - 1;
	}

	return idx;
}

const S32 VIDEO_CARD_MEM_SIZES[6] = { 0x1000000, // 16MB
									  0x2000000, // 32MB
									  0x4000000, // 64MB
									  0x8000000, // 128MB
									  0x10000000, // 256MB
									  0x20000000, // 512MB
									};

const S32 VIDEO_CARD_FRAMEBUFFER_MEM = 0xC00000; // 12MB

void LLViewerImageList::updateMaxResidentTexMem(S32 max, U32 fudge)
{
	// Initialize the image pipeline VRAM settings
	S32 cur_setting = gSavedSettings.getS32("GraphicsCardMemorySetting");
	S32 max_setting = getMaxVideoRamSetting(max);
	if (max >= 0 && max != cur_setting)
	{
		S32 default_setting = getMaxVideoRamSetting(-2); // recommended default
		if (cur_setting >= 0 || max_setting != default_setting)
		{
			gSavedSettings.setS32("GraphicsCardMemorySetting", max_setting);
			return; //listener will reenter this function
		}
		cur_setting = max_setting; // max_setting <= max
	}
	else if (cur_setting < 0)
	{
		S32 default_setting = getMaxVideoRamSetting(-2); // recommended default
		cur_setting = default_setting;
	}
	mVideoMemorySetting = cur_setting;
	// TODO: set available resident texture mem based on use by other subsystems
	// currently max(12MB, VRAM/4) assumed...

	S32 vram_amt = VIDEO_CARD_MEM_SIZES[cur_setting];
	S32 fb_mem = llmax(VIDEO_CARD_FRAMEBUFFER_MEM, vram_amt/4);
	mMaxResidentTexMem = vram_amt - fb_mem - fudge;
	
//	llinfos << "Graphics Card memory set to " << (VIDEO_CARD_MEM_SIZES[cur_setting]>>20)
//			<< " MB" << llendl;
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
	bool res = gTextureFetch->receiveImageHeader(msg->getSender(), id, codec, packets, totalbytes, data_size, data);
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
	bool res = gTextureFetch->receiveImagePacket(msg->getSender(), id, packet_num, data_size, data);
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
	U32 available_memory = memory_info.getPhysicalMemory();
	
	clamp_rescale((F32)available_memory,
				 (F32)(SIXTEEN_MEG * 16),
				 (F32)U32_MAX,
				 (F32)(SIXTEEN_MEG * 4),
				 (F32)(U32_MAX >> 1));
	return available_memory;
}

///////////////////////////////////////////////////////////////////////////////
