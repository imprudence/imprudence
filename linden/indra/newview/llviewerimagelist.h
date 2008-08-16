/** 
 * @file llviewerimagelist.h
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

#ifndef LL_LLVIEWERIMAGELIST_H					
#define LL_LLVIEWERIMAGELIST_H

#include "lluuid.h"
#include "message.h"
#include "llgl.h"
#include "llstat.h"
#include "llviewerimage.h"
#include <list>
#include <set>

const BOOL MIPMAP_YES = TRUE;
const BOOL MIPMAP_NO = FALSE;

const BOOL GL_TEXTURE_YES = TRUE;
const BOOL GL_TEXTURE_NO = FALSE;

const BOOL IMMEDIATE_YES = TRUE;
const BOOL IMMEDIATE_NO = FALSE;

class LLViewerImage;
class LLTextureView;

typedef	void (*LLImageCallback)(BOOL success,
								LLViewerImage *src_vi,
								LLImageRaw* src,
								LLImageRaw* src_aux,
								S32 discard_level,
								BOOL final,
								void* userdata);

class LLViewerImageList : public LLImageProviderInterface
{
        LOG_CLASS(LLViewerImageList);

	friend class LLTextureView;
	
public:
	static BOOL createUploadFile(const LLString& filename, const LLString& out_filename, const U8 codec);
	static LLPointer<LLImageJ2C> convertToUploadFile(LLPointer<LLImageRaw> raw_image);
	static void processImageNotInDatabase( LLMessageSystem *msg, void **user_data );
	static S32 calcMaxTextureRAM();
	static void receiveImageHeader(LLMessageSystem *msg, void **user_data);
	static void receiveImagePacket(LLMessageSystem *msg, void **user_data);

public:
	LLViewerImageList();
	~LLViewerImageList();

	// LLImageProviderInterface
	LLImageGL* getUIImageByID(const LLUUID& id, BOOL clamped = TRUE);

	void init();
	void shutdown();
	void dump();
	void destroyGL(BOOL save_state = TRUE);
	void restoreGL();

	LLViewerImage * getImage(const LLString& filename,
							 const LLUUID &image_id,
							 BOOL usemipmap = TRUE,
							 BOOL level_immediate = FALSE		// Get the requested level immediately upon creation.
							 );
	LLViewerImage * getImage(const LLUUID &image_id,
							 BOOL usemipmap = TRUE,
							 BOOL level_immediate = FALSE		// Get the requested level immediately upon creation.
							 );
	
	LLViewerImage * getImageFromFile(const LLString& filename,
									 const LLUUID &image_id,
									 BOOL usemipmap,
									 BOOL level_immediate,		// Get the requested level immediately upon creation.
									 LLGLint internal_format,
									 LLGLenum primary_format
									 );

	LLViewerImage * getImageFromUUID(const LLUUID &image_id,
									 BOOL usemipmap,
									 BOOL level_immediate,		// Get the requested level immediately upon creation.
									 LLGLint internal_format,
									 LLGLenum primary_format,
									 LLHost request_from_host = LLHost()
									 );
	
	// Request image from a specific host, used for baked avatar textures.
	// Implemented in header in case someone changes default params above. JC
	LLViewerImage* getImageFromHost(const LLUUID& image_id, LLHost host)
		{ return getImageFromUUID(image_id, TRUE, FALSE, 0, 0, host); }

	LLViewerImage *hasImage(const LLUUID &image_id);
	void addImage(LLViewerImage *image);
	void deleteImage(LLViewerImage *image);

	void addImageToList(LLViewerImage *image);
	void removeImageFromList(LLViewerImage *image);

	void updateMovieImage(const LLUUID& image_id, BOOL active);
	void dirtyImage(LLViewerImage *image);
	
	// Using image stats, determine what images are necessary, and perform image updates.
	void updateImages(F32 max_time);

	// Decode and create textures for all images currently in list.
	void decodeAllImages(F32 max_decode_time); 

	void handleIRCallback(void **data, const S32 number);

	void setUpdateStats(BOOL b)			{ mUpdateStats = b; }

	S32	getMaxResidentTexMem() const	{ return mMaxResidentTexMem; }
	S32	getVideoMemorySetting() const	{ return mVideoMemorySetting; }
	S32 getNumImages()					{ return mImageList.size(); }

	static S32 getMaxVideoRamSetting(S32 max = -1);
	void updateMaxResidentTexMem(S32 max = -1, U32 fudge = 0);
	
	void doPreloadImages();
	void doPrefetchImages();

private:
	LLViewerImage* preloadImage(const LLString& filename, const LLUUID &image_set_id, BOOL use_mips);
	void updateImagesDecodePriorities();
	F32  updateImagesCreateTextures(F32 max_time);
	F32  updateImagesFetchTextures(F32 max_time);
	void updateImagesMediaStreams();
	void updateImagesUpdateStats();
	
public:
	typedef std::set<LLPointer<LLViewerImage> > image_list_t;	
	image_list_t mLoadingStreamList;
	image_list_t mCreateTextureList;
	image_list_t mCallbackList;

	// Note: just raw pointers because they are never referenced, just compared against
	std::set<LLViewerImage*> mDirtyTextureList;
	
	BOOL mForceResetTextureStats;
    
private:
	typedef std::map< LLUUID, LLPointer<LLViewerImage> > uuid_map_t;
	uuid_map_t mUUIDMap;
	LLUUID mLastUpdateUUID;
	LLUUID mLastFetchUUID;
	
	typedef std::set<LLPointer<LLViewerImage>, LLViewerImage::Compare> image_priority_list_t;	
	image_priority_list_t mImageList;

	typedef std::vector<LLPointer<LLViewerImage> > callback_data_t;
	typedef std::set< callback_data_t* > callback_data_list_t;
	callback_data_list_t mIRCallbackData;

	BOOL mUpdateStats;
	S32	mMaxResidentTexMem;
	S32	mVideoMemorySetting;
	LLFrameTimer mForceDecodeTimer;
	
	LLUUID mMovieImageUUID;
	U8  mMovieImageHasMips;

	std::vector<LLPointer<LLViewerImage> > mPreloadedImages;

public:
	static U32 sTextureBits;
	static U32 sTexturePackets;

	static LLStat sNumImagesStat;
	static LLStat sNumRawImagesStat;
	static LLStat sGLTexMemStat;
	static LLStat sGLBoundMemStat;
	static LLStat sRawMemStat;
	static LLStat sFormattedMemStat;

private:
	static S32 sNumImages;
	static void (*sUUIDCallback)(void**, const LLUUID &);
};

const BOOL GLTEXTURE_TRUE = TRUE;
const BOOL GLTEXTURE_FALSE = FALSE;
const BOOL MIPMAP_TRUE = TRUE;
const BOOL MIPMAP_FALSE = FALSE;

extern LLViewerImageList gImageList;

#endif
