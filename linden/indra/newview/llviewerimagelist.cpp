/** 
 * @file llviewerimagelist.cpp
 * @brief Object for managing the list of images within a region
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

#include "llvfs.h"
#include "llvfile.h"
#include "llvfsthread.h"
#include "message.h"

#include "llagent.h"
#include "llviewercontrol.h"
#include "lltexturetable.h" // For looking up names from uuid's.
#include "llviewerimage.h"
#include "llviewerregion.h"
#include "viewer.h"
#include "llsys.h"
#include "llpaneldisplay.h" // for LL_MAX_VRAM_INDEX

void (*LLViewerImageList::sUUIDCallback)(void **, const LLUUID&) = NULL;

const U32 SIXTEEN_MEG = 0x1000000;
const F32 MAX_IMAGE_PRIORITY = 100000000.f;

U32 LLViewerImageList::sTextureBits = 0;
U32 LLViewerImageList::sTexturePackets = 0;

struct CodecExtMap
{
	CodecExtMap( const char* ext, U8 codec ) : mExt( ext ), mCodec( codec ) {}
	const char* mExt;
	U8 mCodec;
};

// Note: Keep codec extensions in order of likelihood the image
// will be found.
const S32 CODEC_EXT_MAP_COUNT = 2;
const CodecExtMap CODEC_EXT_MAP[CODEC_EXT_MAP_COUNT] =
{
	CodecExtMap( "_07", IMG_CODEC_J2C ),
	CodecExtMap( ".tga", IMG_CODEC_TGA )
};

const U32 MIN_AVAIL_MEM_FOR_DECODE = 1048576;

// HACK: We have to try to allocate a constant fraction of this
// memory for AGP.  This isn't worthwhile on 128 meg cards.
// So for 128 meg cards, only alloc 64 megs.
const S32 VIDEO_CARD_MEM_SIZES[6] = { 0x1000000, // 16MB
									  0x2000000, // 32MB
									  0x4000000,  // 64MB+
									  0x8000000, // 128MB
									  0x10000000, // 256MB
									  0x20000000, // 512MB
									};

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

//static
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

	// This stuff is global! Bad behavior if more than one image list.

	// Set the fallback GL texture to smoke...
 	LLViewerImage::sSmokeImagep = getImage(IMG_SMOKE, TRUE, TRUE);
	// Set the fallback GL texture to gray with a white border...
#if 0
	LLViewerImage* imagep = new LLViewerImage(IMG_DEFAULT, TRUE);
	LLViewerImage::sDefaultImagep = imagep;
	const S32 dim = 128;
	const S32 border = 2;
	LLPointer<LLImageRaw> image_raw = new LLImageRaw(dim,dim,3);
	U8* data = image_raw->getData();
	for (S32 i = 0; i<dim; i++)
	{
		for (S32 j = 0; j<dim; j++)
		{
			if (i<border || j<border || i>=(dim-border) || j>=(dim-border))
			{
				*data++ = 0xff;
				*data++ = 0xff;
				*data++ = 0xff;
			}
			else
			{
				*data++ = 0x7f;
				*data++ = 0x7f;
				*data++ = 0x7f;
			}
		}
	}
	imagep->createGLTexture(0, image_raw);
	image_raw = NULL;
	addImage(imagep);
	imagep->dontDiscard();
#else
 	LLViewerImage::sDefaultImagep = getImage(IMG_DEFAULT, TRUE, TRUE);
#endif


	mUpdateStats = TRUE;

	// Update how much texture RAM we're allowed to use.
	updateMaxResidentTexMem();

	mMovieImageHasMips = FALSE;
}

void LLViewerImageList::doPreloadImages()
{
	llinfos << "Preloading images..." << llendl;
	
	// Set the "missing asset" image
	LLViewerImage::sMissingAssetImagep = preloadImage("missing_asset.tga" , LLUUID::null, TRUE);

	// Set the "white" image
	LLViewerImage::sWhiteImagep = preloadImage("white.tga", LLUUID::null, TRUE);;

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
	preloadImage("tab_top_blue.tga", LLUUID::null, FALSE);
	preloadImage("tab_top_selected_blue.tga", LLUUID::null, FALSE);
	preloadImage("tool_dozer.tga", LLUUID::null, FALSE);
	preloadImage("tool_dozer_active.tga", LLUUID::null, FALSE);
	preloadImage("tool_zoom.tga", LLUUID::null, FALSE);
	preloadImage("tool_zoom_active.tga", LLUUID::null, FALSE);
	preloadImage("white.tga", LLUUID::null, TRUE);
}

///////////////////////////////////////////////////////////////////////////////

LLViewerImageList::~LLViewerImageList()
{
	llassert(mIRCallbackData.empty());
}

void LLViewerImageList::shutdown()
{
	// Clean up potential callback data
	// mIRCallbackData is now stl and will clean itself up

	//
	// Clean up "loaded" callbacks.
	//
	mCallbackList.clear();

	// Clean up preloaded images
	mPreloadedImages.clear();
	
	//
	// If we're working on decoding an image, finish it off so we can clean it up.
	//
	LLViewerImage *imagep = mCurrentDecodeImagep;
	if (imagep)
	{
		imagep->abortDecode();
		imagep->destroyRawImage();
		mCurrentDecodeImagep = NULL;
	}

	// Flush all of the references
	mLoadingStreamList.clear();

	mUUIDMap.clear();

	// This stuff is global!
	LLViewerImage::sDefaultImagep = NULL;

	mImageList.clear();
}

void LLViewerImageList::dump()
{
	llinfos << "LLViewerImageList::dump()" << llendl;
	for (image_list_t::iterator it = mImageList.begin(); it != mImageList.end(); ++it)
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

LLViewerImage * LLViewerImageList::getImage(const LLString& filename,
											const LLUUID &image_set_id,
											BOOL usemipmaps,
											BOOL level_immediate)
{
	return getImageFromFile(filename, image_set_id, usemipmaps, level_immediate, 0, 0);
}

LLViewerImage * LLViewerImageList::getImageFromFile(const LLString& filename,
													const LLUUID &image_set_id,
													BOOL usemipmaps,
													BOOL level_immediate,
													LLGLint	internal_format,
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
		llwarns << "Unable to find inage " << filename << " in gViewerArt" << llendl;
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
	
	// First see if we already have this image loaded.
	LLPointer<LLViewerImage> imagep = hasImage(image_id);

	if (imagep.isNull())
	{
		// No image loaded.  Try to read the filename given.
		bool success = false;
		if (!filename.empty())
		{
			// This is strictly for local .tga files not in the static VFS
			LLString image_file = gDirUtilp->getExpandedFilename(LL_PATH_TOP_SKIN, filename);
			imagep = new LLViewerImage(image_id, usemipmaps);
			LLPointer<LLImageRaw> image_raw = new LLImageRaw(image_file);
			if ( image_raw->getDataSize() > 0 )
			{
				imagep->createGLTexture(0, image_raw);
				image_raw = NULL;

				if (usemipmaps == FALSE)
				{
					// num mipmaped textures are almost always clamped, so clamp by default
					imagep->bind();
					imagep->setClamp(TRUE, TRUE);
				}
				
				if (internal_format && primary_format)
				{
					imagep->setExplicitFormat(internal_format, primary_format);
				}

				addImage(imagep);

				if (level_immediate)
				{
					imagep->dontDiscard();
				}

				success = true;
			}
			else
			{
				imagep = NULL;
			}
		}

		if (!success)
		{
			// We couldn't load from a file.  Try the VFS.
			imagep = getImageFromUUID(image_id, usemipmaps, level_immediate, 
									  internal_format, primary_format, LLHost());
		}
	}

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
		}

		// if rendering enabled, actually try to load this image
		if (!gNoRender)
		{
			// First check the local image cache to see if it's there.
			if (imagep->loadLocalImage(image_id))
			{
// 				llinfos << "Loading Local Image: " << image_id
// 						<< llformat(" MIP:%d IMM:%d",usemipmaps,level_immediate)
// 						<< llendl;
				if( level_immediate )
				{
					if (imagep->needsDecode())
					{
						imagep->decodeImage(0.f); // getImage (local image)
					}

					if (imagep->getNeedsCreateTexture())
					{
						imagep->createTexture();
					}
				}
			}
			else
			{
				// if we don't have this locally, we'll want to start on the highest discard
				if (!imagep->getDontDiscard())
				{
					imagep->setDesiredDiscardLevel(imagep->getMaxDiscardLevel());
				}

				imagep->startVFSLoad();
			}
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
	
#if 0
	// Add this image to the viewer image list.
	if (new_image->getDecodePriority() == 0.0f)
	{
		new_image->setDecodePriority(MAX_IMAGE_PRIORITY); // Initially put in front of list
	}
#endif
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

void image_request_callback(void **data, S32 number)
{
	gImageList.handleIRCallback(data, number);
}

void LLViewerImageList::handleIRCallback(void **data, const S32 number)
{
	callback_data_t* requested_images = (callback_data_t*)data;
	if (number == LL_ERR_TCP_TIMEOUT)
	{
		for (callback_data_t::iterator iter = requested_images->begin();
			 iter != requested_images->end();)
		{
			LLViewerImage* image = *iter++;
			image->mRequested = FALSE;
			image->mRequestedDiscardLevel = -1; // Indicates we need to re-request this
		}
	}

	// Delete and remove from our list of callback data
	delete requested_images;
	llverify(mIRCallbackData.erase(requested_images) == 1);
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

void LLViewerImageList::updateImages(const F32 decode_time_max)
{
	sNumImagesStat.addValue(sNumImages);
	sNumRawImagesStat.addValue(LLImageRaw::sRawImageCount);
	sGLTexMemStat.addValue(LLImageGL::sGlobalTextureMemory/(1024.f*1024.f));
	sGLBoundMemStat.addValue(LLImageGL::sBoundTextureMemory/(1024.f*1024.f));
	sRawMemStat.addValue(LLImageRaw::sGlobalRawMemory/(1024.f*1024.f));
	sFormattedMemStat.addValue(LLImageFormatted::sGlobalFormattedMemory/(1024.f*1024.f));

	updateImagesDecodePriorities();
	updateImagesSendRequests();
	
	if (gGLManager.mIsDisabled)
	{
		// We don't want to run this part of the texture system while we don't have
		// a GL context - we COULD probably do some of it, but that's tricky - djs 10/29/03
		return;
	}

	updateImagesDecodeTextures(decode_time_max);
	updateImagesMediaStreams();
	updateImagesPollVFS();
	updateImagesUpdateStats();
}


void LLViewerImageList::updateImagesDecodePriorities()
{
	// Update the decode priority for N images each frame
	{
		const size_t max_update_count = 256;
		S32 update_counter = llmin(max_update_count, mUUIDMap.size());
		uuid_map_t::iterator iter = mUUIDMap.upper_bound(mLastUpdateUUID);
		while(update_counter > 0)
		{
			if (iter == mUUIDMap.end())
			{
				iter = mUUIDMap.begin();
			}
			mLastUpdateUUID = iter->first;
			LLPointer<LLViewerImage> imagep = iter->second;
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
			iter++;
			update_counter--;
		}
	}
}

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

void LLViewerImageList::updateImagesSendRequests()
{
	// Send requests for images based on priority.
	{
		S32 request_count = 0;
		S32 request_packets_sent = 0;
		S32 update_count = 0;
	
		callback_data_t *requested_images = NULL;

		// Baked texture images may live on a separate host. JC
		std::vector< LLPointer<LLViewerImage> > images_on_other_hosts;
		LLHost agent_host = gAgent.getRegionHost();

		for (image_list_t::iterator iter = mImageList.begin();
			 iter != mImageList.end(); )
		{
			image_list_t::iterator curiter = iter++;
			LLPointer<LLViewerImage> imagep = *curiter;

			if (imagep->mIsMediaTexture)
			{
				continue; // skip
			}

			F32 decode_priority = imagep->getDecodePriority();
			
			update_count++;
			if (mUpdateStats == FALSE &&
				update_count >= IMAGES_MIN_UPDATES &&
				decode_priority < MAX_IMAGE_PRIORITY)
			{
				break;
			}
			
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
				if (!(imagep->isDecoding()))
				{
					if (imagep->mLastReferencedTimer.getElapsedTimeF32() > LAZY_FLUSH_TIMEOUT)
					{
						if (imagep->mStreamFile && !imagep->mStreamFile->isReadComplete())
						{
							llwarns << "Stream file is still reading data, delaying flush!" << llendl;
						}
						else
						{
							// Remove the unused image from the image list
							deleteImage(imagep);
							imagep = NULL; // should destroy the image
							continue;
						}
					}
				}
			}
			else
			{
				imagep->mLastReferencedTimer.reset();
			}

			if (decode_priority <= 0)
			{
				continue;
			}
			if (imagep->isMissingAsset())
			{
				continue;
			}	
			if (imagep->checkPacketData())
			{
				// New packets have been processed, re-evaluate next time
				continue;
			}
			if (request_packets_sent >= IMAGES_MAX_PACKET_UPDATES)
			{
				continue;
			}
			if (!gAgent.getRegion())
			{
 				//llinfos << "Skipping request for " << imagep->getID() << " while waiting for a region" << llendl;
				continue;
			}

			F32 old_priority = imagep->mRequestedDownloadPriority;
			S32 current_discard = imagep->getDiscardLevel();
			S32 desired_discard = imagep->getDesiredDiscardLevel();

			if (current_discard >= 0 && current_discard <= desired_discard)
			{
				continue;
			}
			
			if (imagep->mRequestTime.getElapsedTimeF32() <= RESEND_IMAGE_REQUEST_TIME)
			{
				// Ignore < 20% difference, or no change in requested discard level
				if ((decode_priority > old_priority * .8f && decode_priority < old_priority * 1.25f) &&
					(desired_discard == imagep->mRequestedDiscardLevel))
				{
					continue;
				}			
			}
			
			// Send the request
			{			
				// Baked avatar textures may live on other hosts. JC
				LLHost target_host = imagep->getTargetHost();

				// This file is in the static VFS, we don't ever need to request it from the network.
				if (imagep->mInStaticVFS && imagep->mFormattedFlushed)
				{
					// Unneeded? JC 8/2006
					imagep->mRequestedDiscardLevel = desired_discard;
					imagep->mRequestedDownloadPriority = decode_priority;
					
					// It's in the static VFS but not loaded, just load it from disk instead of sending a request.
					imagep->startVFSLoad();
				}
				else if (target_host.isOk() && target_host != agent_host)
				{
					// This is a special texture to request off a sim other than
					// the one the agent is on.  We'll deal with it later.
					images_on_other_hosts.push_back(imagep);
				}
				else
				{
					imagep->mRequestedDiscardLevel = desired_discard;
					imagep->mRequestedDownloadPriority = decode_priority;
					
					if (0 == request_count)
					{
						// Create a message if this is the first image request.
						gMessageSystem->newMessageFast(_PREHASH_RequestImage);
						gMessageSystem->nextBlockFast(_PREHASH_AgentData);
						gMessageSystem->addUUIDFast(
							_PREHASH_AgentID, gAgent.getID());
						gMessageSystem->addUUIDFast(
							_PREHASH_SessionID, gAgent.getSessionID());
						requested_images = new callback_data_t;
						// verify that requested_images is placed uniquely in the list
						llverify((mIRCallbackData.insert(requested_images)).second);
					}

					requested_images->push_back(imagep);
					request_count++;

					gMessageSystem->nextBlockFast(_PREHASH_RequestImage);
					S32 packet = imagep->getLastPacket() + 1;
					gMessageSystem->addUUIDFast(_PREHASH_Image, imagep->getID());
					gMessageSystem->addS8Fast(_PREHASH_DiscardLevel, (S8)desired_discard);
					gMessageSystem->addF32Fast(_PREHASH_DownloadPriority, decode_priority);
					gMessageSystem->addU32Fast(_PREHASH_Packet, packet);
					U8 type = get_image_type(imagep, target_host);
					gMessageSystem->addU8Fast(_PREHASH_Type, type);

					lldebugst(LLERR_IMAGE)
						<< "IMAGE REQUEST: " << imagep->getID().getString()
						<< " discard: " << desired_discard
						<< " old_pri: " << old_priority
						<< " dld_pri: " << decode_priority
						<< " dec_pri: " << imagep->getDecodePriority()
						<< llendl;
						
				}

				imagep->mRequested = TRUE;
				imagep->mRequestTime.reset();

				if (request_count >= IMAGES_PER_REQUEST)
				{
					// IMAGES_PER_REQUEST packets combined, send packet.
					gMessageSystem->sendSemiReliable(gAgent.getRegion()->getHost(), image_request_callback, (void **)requested_images);

					requested_images = NULL;
					request_count = 0;
					++request_packets_sent;
				}
			}
		}
		
		if (request_count != 0)
		{
			// fill in the unused requested_images w/ NULL
			gMessageSystem->sendSemiReliable(gAgent.getRegion()->getHost(), image_request_callback, (void **)requested_images);
			requested_images = NULL;
			++request_packets_sent;
		}
		
		// We might have picked up some images on other hosts.
		if (!images_on_other_hosts.empty())
		{
			// llinfos << "TAT: images_on_other_hosts " << images_on_other_hosts.size() << llendl;

			std::sort(images_on_other_hosts.begin(), images_on_other_hosts.end(), LLViewerImage::CompareByHostAndPriority());

			LLMessageSystem* msg = gMessageSystem;
			LLHost current_host = images_on_other_hosts[0]->getTargetHost();
			request_count = 0;

			for (std::vector<LLPointer<LLViewerImage> >::iterator it = images_on_other_hosts.begin();
				it != images_on_other_hosts.end();
				++it)
			{
				LLPointer<LLViewerImage> imagep = *it;
				
				F32 decode_priority = imagep->getDecodePriority();
				S32 desired_discard = imagep->getDesiredDiscardLevel();
				
				imagep->mRequestedDiscardLevel = desired_discard;
				imagep->mRequestedDownloadPriority = decode_priority;
				
				if ((current_host != imagep->getTargetHost() || request_count >= IMAGES_PER_REQUEST) && request_count)
				{
					// llinfos << "TAT: Sending " << request_count << " image requests for host: " << current_host << llendl;

					// Need to flush to current host.
					gMessageSystem->sendSemiReliable(current_host, image_request_callback, (void **)requested_images);
					
					requested_images = NULL;
					current_host = imagep->getTargetHost();
					request_count = 0;
				}

				if (request_count == 0)
				{
					// Start a packet and build a new callback list for dropped
					// packet handler.
					msg->newMessageFast(_PREHASH_RequestImage);
					msg->nextBlockFast(_PREHASH_AgentData);
					msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
					msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
					requested_images = new callback_data_t;
					mIRCallbackData.insert(requested_images);
					current_host = imagep->getTargetHost();
				}
							
				requested_images->push_back(imagep);
				request_count++;
					
				msg->nextBlockFast(_PREHASH_RequestImage);
				S32 packet = imagep->getLastPacket() + 1;
				msg->addUUIDFast(_PREHASH_Image, imagep->getID());
				msg->addS8Fast(_PREHASH_DiscardLevel, (S8)desired_discard);
				msg->addF32Fast(_PREHASH_DownloadPriority, decode_priority);
				msg->addU32Fast(_PREHASH_Packet, packet);
				U8 type = get_image_type(imagep, current_host);
				gMessageSystem->addU8Fast(_PREHASH_Type, type);
				
				if (!gMessageSystem->checkCircuitAlive(current_host))
				{
					llinfos << "TAT: Image request for dead circuit " << current_host << ", " << imagep->getID() << llendl;
					imagep->setTargetHost(agent_host);
				}

				if (!gMessageSystem->checkCircuitAlive(current_host))
				{
					llinfos << "TAT: Image request for dead circuit " << current_host << ", " << imagep->getID() << llendl;
					imagep->setTargetHost(agent_host);
				}

				imagep->mRequested = TRUE;
				imagep->mRequestTime.reset();				
			}

			if (request_count != 0)
			{
				// fill in the unused requested_images w/ NULL
				msg->sendSemiReliable(current_host, image_request_callback, (void **)requested_images);
				requested_images = NULL;
			}
		}
	}
}



void LLViewerImageList::updateImagesDecodeTextures(F32 decode_time_max)
{
	if (gNoRender) return;

		LLTimer image_op_timer;

		BOOL done_one = FALSE;
		image_op_timer.reset();

		S32 create_count = 0;

		// added by IW to help track down a bug
		stop_glerror();

		//
		// Create GL textures for all textures that need them (images which have been
		// decoded, but haven't been pushed into GL).
		//
		{
 		LLFastTimer t(LLFastTimer::FTM_IMAGE_CREATE);

		for (image_list_t::iterator iter = mImageList.begin();
			 iter != mImageList.end(); )
		{
			LLPointer<LLViewerImage> imagep = *iter++;
			if (imagep->getNeedsCreateTexture())
			{
				create_count++;
				imagep->createTexture();
				if (decode_time_max != 0.f)
				{
					if (image_op_timer.getElapsedTimeF32() > decode_time_max)
					{
						lldebugst(LLERR_IMAGE) << "Broke out of create texture!" << llendl;
						break;
					}
				}
			}
		}
		}
		
		//
		// Work on decoding any image that's partially decoded, first
		//
		// Everything after here is time-sliced
		//

		if (mCurrentDecodeImagep.notNull()
			&& mCurrentDecodeImagep->needsDecode()
			&& mCurrentDecodeImagep->isDecoding())
		{
			//llinfos << "Continue decoding " << mCurrentDecodeImagep->getID() << llendl;
			LLFastTimer t(LLFastTimer::FTM_IMAGE_DECODE);
			if (decode_time_max != 0.f)
			{
				if (done_one && image_op_timer.getElapsedTimeF32() >= decode_time_max)
				{
					lldebugst(LLERR_IMAGE) << "Broke out of partial decode!" << llendl;
				}
				else
				{
					F32 decode_time = decode_time_max - image_op_timer.getElapsedTimeF32();
					decode_time = llmax(decode_time, .0001f); // min .1 ms
					mCurrentDecodeImagep->decodeImage(decode_time); // Partial decode
					done_one = TRUE;
				}
			}
			else
			{
				mCurrentDecodeImagep->decodeImage(0.0f); // Partial decode
				done_one = TRUE;
			}
		}
		
		//
		// Reprioritize any image that we just finished decoding
		//
		if (mCurrentDecodeImagep.notNull()
			&& (!mCurrentDecodeImagep->needsDecode()
				|| !mCurrentDecodeImagep->isDecoding()))
		{
			// Reprioritize this image
			if (mCurrentDecodeImagep->mInImageList)
			{
				removeImageFromList(mCurrentDecodeImagep);
				mCurrentDecodeImagep->setDecodePriority();
				addImageToList(mCurrentDecodeImagep);
			}
			mCurrentDecodeImagep = NULL;
		}

		//
		// At this point, we're going to check out the status of anything that's
		// on our callback list.  Instead of calling the callback lists from a
		// billion different places, we're ONLY going to handle them here.
		//
		// Do this here so if the callbacks take a lot of time, it counts
		// against our decode timer totals

		// For right now, just be really lame and just iterate through all images.
		// This WILL be optimized soon.

		if (mUpdateStats)
		{
			// This is somewhat intensive, and it doesn't need to happen
			// immediately, so only do it when we update stats.
			for (image_callback_list_t::iterator iter = mCallbackList.begin();
				 iter != mCallbackList.end(); )
			{
				LLViewerImage* image = *iter++;
				// Do stuff to handle callbacks, update priorities, etc.
				image->doLoadedCallbacks();
			}
		}

		//
		// Decode as many images as we can at this point.
		// If we're in the middle of finishing up one still,
		// don't decode any more textures
		//
		if (mCurrentDecodeImagep.isNull() || mCurrentDecodeImagep->getBoostLevel())
		{
			LLFastTimer t(LLFastTimer::FTM_IMAGE_DECODE);
			do
			{
				BOOL did_decode = FALSE;
				BOOL have_map_image = FALSE;
				for (image_list_t::iterator iter = mImageList.begin();
					 iter != mImageList.end(); )
				{
					image_list_t::iterator curiter = iter++;
					LLPointer<LLViewerImage> imagep = *curiter;
					if (imagep->needsDecode())
					{
						if (decode_time_max != 0.f)
						{
							if (!imagep->getBoostLevel() && done_one &&
								image_op_timer.getElapsedTimeF32() >= decode_time_max)
							{
								break;
							}
							F32 decode_time = decode_time_max - image_op_timer.getElapsedTimeF32();
							decode_time = llmax(decode_time, .0001f); // min .1 ms
							imagep->decodeImage(decode_time);
						}
						else
						{
							imagep->decodeImage(0.0f);
						}
					
						if (imagep->needsDecode())
						{
							mCurrentDecodeImagep = imagep;
						}
						else
						{
							// Reprioritize this image
							removeImageFromList(imagep);
							imagep->setDecodePriority();
							addImageToList(imagep);
						
							mCurrentDecodeImagep = NULL;
						}
						done_one = TRUE;
						did_decode = TRUE;
					}
					if (imagep->getBoostLevel() >= LLViewerImage::BOOST_MAP)
					{
						have_map_image = TRUE;
					}
					else if (have_map_image)
					{
						break; // skip other images if we are decoding map images
					}
				}
				if (!did_decode)
				{
					break;
				}
			} while (image_op_timer.getElapsedTimeF32() < decode_time_max);
		}
}

void LLViewerImageList::updateImagesMediaStreams()
{
	if (gNoRender) return;
	
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
							llassert(!viewerImage->getUseMipMaps());
						
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

						LLImageRaw* rawImage = media_engine->getImageRaw();

						if ( rawImage )
						{
							((LLImageGL*)viewerImage)->setSubImage(rawImage, 0, 0,
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

void LLViewerImageList::updateImagesPollVFS()
{
		// Sigh, VFS stuff has to be polled.  The VFS really needs some sort
		// of mechanism to avoid this issue.
		for (image_loading_list_t::iterator iter = mLoadingStreamList.begin();
			 iter != mLoadingStreamList.end();)
		{
			image_loading_list_t::iterator curiter = iter++;
			LLViewerImage *imagep = *curiter;
			imagep->loadStreamFile();
			if (!imagep->mStreamFile)
			{
				iter = mLoadingStreamList.erase(curiter);
			}
		}
}

void LLViewerImageList::updateImagesUpdateStats()
{
	if (mUpdateStats)
	{
		for (image_list_t::iterator iter = mImageList.begin();
			 iter != mImageList.end(); )
		{
			LLViewerImage* imagep = *iter++;
			imagep->resetTextureStats(mForceResetTextureStats);
		}
#if 0
		S32 needs_decode_count = 0;
		for (image_list_t::iterator iter = mImageList.begin();
			 iter != mImageList.end(); )
		{
			LLViewerImage* imagep = *iter++;
			// count priority images in need of decode (10000 ~= 100x100 pixels)
			if (imagep->getDecodePriority() > 10000.f && (imagep->needsDecode() || imagep->getNeedsCreateTexture()))
			{
				needs_decode_count++;
			}
		}

		// If we have a lot of priority decodes pending, take some time to decode them
		const S32 force_decode_count = 20;
		const F32 force_decode_time = 2.f; // seconds
		const F32 force_decode_delay = 30.f; // seconds
		if (needs_decode_count > force_decode_count && mForceDecodeTimer.hasExpired())
		{
			decodeAllImages(force_decode_time); // spend some time decoding images
			mForceDecodeTimer.setTimerExpirySec(force_decode_delay); // wait 10 seconds
		}
#endif
		mUpdateStats = FALSE;
		mForceResetTextureStats = FALSE;
	}
}

void LLViewerImageList::decodeAllImages(F32 max_decode_time)
{
	LLTimer timer;
	if(!gNoRender)
	{
		for (image_list_t::iterator iter = mImageList.begin();
			 iter != mImageList.end(); )
		{
			LLViewerImage* imagep = *iter++;
			if (imagep->needsDecode())
			{
				imagep->decodeImage(0.f); // LLViewerImageList::decodeAllImages
			}
			if (max_decode_time > 0.0f && timer.getElapsedTimeF32() > max_decode_time)
			{
				break;
			}
		}

		for (image_list_t::iterator iter = mImageList.begin();
			 iter != mImageList.end(); )
		{
			LLViewerImage* imagep = *iter++;
			if (imagep->getNeedsCreateTexture())
			{
				imagep->createTexture();
			}
		}
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

	raw_image->biasedScaleToPowerOfTwo(LLViewerImage::MAX_IMAGE_SIZE_DEFAULT);

	LLPointer<LLImageJ2C> compressedImage = new LLImageJ2C;

	compressedImage->setRate(0.f);
	compressedImage->encode(raw_image);
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

//static
S32 LLViewerImageList::getMaxVideoRamSetting(S32 max)
{
	const U32 vram_settings[] = { 16, 32, 64, 128, 256, 512 };

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
	llinfos << "*** DETECTED " << system_ram/(double)(1<<20) << " MB of system memory." << llendl; // TomY TESTING DNCI
	if (max == -2)
	{
		max_vram = llmin(max_vram, (U32)(system_ram/2)); // max recommended setting
	}
	else
	{
		max_vram = llmin(max_vram, (U32)((F32)system_ram/1.5f));
	}
	S32 idx;
	for (idx=0; idx<=5; idx++)
	{
		if (idx == max)
			break;
		if ((vram_settings[idx] << 20) > max_vram)
		{
			idx--;
			break;
		}
	}
	return idx;
}

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
	// currently 12MB assumed...
	
	mMaxResidentTexMem = VIDEO_CARD_MEM_SIZES[cur_setting] - 0xC00000 - fudge;  // - 12MB
	mMaxResidentTexMem -= mMaxResidentTexMem/8;
	
	llinfos << "Graphics Card memory set to " << (VIDEO_CARD_MEM_SIZES[cur_setting]>>20)
			<< " MB" << llendl;
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
		image->setIsMissingAsset( TRUE );
	}
}
