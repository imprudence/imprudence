/** 
 * @file llviewerimage.h
 * @brief Object for managing images and their textures
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

#ifndef LL_LLVIEWERIMAGE_H					
#define LL_LLVIEWERIMAGE_H

#include "llimagegl.h"
#include "lltimer.h"
#include "llframetimer.h"
#include "llhost.h"

#include <map>
#include <list>

class LLFace;
#define MIN_VIDEO_RAM_IN_MEGA_BYTES    32
#define MAX_VIDEO_RAM_IN_MEGA_BYTES    512 // 512MB max for performance reasons.

class LLViewerImage;

typedef	void	(*loaded_callback_func)( BOOL success, LLViewerImage *src_vi, LLImageRaw* src, LLImageRaw* src_aux, S32 discard_level, BOOL final, void* userdata );

class LLVFile;
class LLMessageSystem;
class LLVOVolume;
 
class LLLoadedCallbackEntry
{
public:
	LLLoadedCallbackEntry(loaded_callback_func cb,
						  S32 discard_level,
						  BOOL need_imageraw, // Needs image raw for the callback
						  void* userdata ) 
		: mCallback(cb),
		  mLastUsedDiscard(MAX_DISCARD_LEVEL+1),
		  mDesiredDiscard(discard_level),
		  mNeedsImageRaw(need_imageraw),
		  mUserData(userdata)
	{
	}

	loaded_callback_func	mCallback;
	S32						mLastUsedDiscard;
	S32						mDesiredDiscard;
	BOOL					mNeedsImageRaw;
	void*					mUserData;
};

class LLTextureBar;

//=====================================
struct LLViewerImageBoostLevel
{
	enum
	{
		BOOST_NONE 			= 0,
		BOOST_AVATAR_BAKED	= 1,
		BOOST_AVATAR		= 2,
		BOOST_CLOUDS		= 3,
		BOOST_SCULPTED      = 4,
		
		BOOST_HIGH 			= 5,
		BOOST_TERRAIN		, // has to be high priority for minimap / low detail
		BOOST_SELECTED		,
		BOOST_HUD			,
		BOOST_AVATAR_BAKED_SELF	,
		BOOST_UI			,
		BOOST_PREVIEW		,
		BOOST_MAP			,
		BOOST_MAP_VISIBLE	,
		BOOST_AVATAR_SELF	,// needed for baking avatar
		BOOST_MAX_LEVEL,

		//LLImageGLCategory
		TEXLAYER_BUMP = BOOST_MAX_LEVEL,
		AVATAR_SCRATCH_TEX,
		FONT,
		BUMP_IMAGE,
		DYNAMIC_TEX,
		TEXLAYER_CACHE,
		MEDIA,
		ATLAS,
		OTHER,
		MAX_GL_IMAGE_CATEGORY
	};
};
//=====================================
class LLViewerImage : public LLImageGL
{
        LOG_CLASS(LLViewerImage);

	friend class LLTextureBar; // debug info only
	friend class LLTextureView; // debug info only
	
public:
	static void initClass();
	static void cleanupClass();
	static void updateClass(const F32 velocity, const F32 angular_velocity);

	static LLViewerImage * getImage(const LLUUID &image_id);
		// lightweight wrapper for gImageList.getImage()


	struct Compare
	{
		// lhs < rhs
		bool operator()(const LLPointer<LLViewerImage> &lhs, const LLPointer<LLViewerImage> &rhs) const
		{
			const LLViewerImage* lhsp = (const LLViewerImage*)lhs;
			const LLViewerImage* rhsp = (const LLViewerImage*)rhs;
			// greater priority is "less"
			const F32 lpriority = lhsp->getDecodePriority();
			const F32 rpriority = rhsp->getDecodePriority();
			if (lpriority > rpriority) // higher priority
				return true;
			if (lpriority < rpriority)
				return false;
			return lhsp < rhsp;
		}
	};

	struct CompareByHostAndPriority
	{
		// lhs < rhs
		bool operator()(const LLPointer<LLViewerImage> &lhs, const LLPointer<LLViewerImage> &rhs) const
		{
			const LLViewerImage* lhsp = (const LLViewerImage*)lhs;
			const LLViewerImage* rhsp = (const LLViewerImage*)rhs;
			if (lhsp->mTargetHost != rhsp->mTargetHost)
				return lhsp->mTargetHost < rhsp->mTargetHost;

			const F32 lpriority = lhsp->getDecodePriority();
			const F32 rpriority = rhsp->getDecodePriority();
			if (lpriority != rpriority)
				return lpriority > rpriority; // greater priority is "less"

			return lhsp < rhsp;
		}
	};

	struct CompareForRemoval
	{
		// lhs < rhs
		bool operator()(const LLPointer<LLViewerImage> &lhs, const LLPointer<LLViewerImage> &rhs) const
		{
			const LLViewerImage* lhsp = (const LLViewerImage*)lhs;
			const LLViewerImage* rhsp = (const LLViewerImage*)rhs;
			// compare bind time
			if (lhsp->mLastBindTime < rhsp->mLastBindTime) // older
				return true;
			if (lhsp->mLastBindTime > rhsp->mLastBindTime)
				return false;
			if (lhsp->getDiscardLevel() < rhsp->getDiscardLevel()) // larger
				return true;
			if (lhsp->getDiscardLevel() > rhsp->getDiscardLevel())
				return false;
			return lhsp < rhsp;
		}
	};

	struct CompareForWorstVisibility 
	{
		// lhs < rhs
		bool operator()(const LLPointer<LLViewerImage> &lhs, const LLPointer<LLViewerImage> &rhs) const
		{
			const LLViewerImage* lhsp = (const LLViewerImage*)lhs;
			const LLViewerImage* rhsp = (const LLViewerImage*)rhs;
			F32 lhsvis = ((lhsp->getWidth() * lhsp->getHeight()) / lhsp->mMaxVirtualSize);
			F32 rhsvis = ((rhsp->getWidth() * rhsp->getHeight()) / rhsp->mMaxVirtualSize);
			if (lhsvis > rhsvis) // fewer relative visible pixels
				return true;
			if (rhsvis < lhsvis)
				return false;
			return lhsp < rhsp;
		}
	};
	
	enum
	{
		MAX_IMAGE_SIZE_DEFAULT = 1024,
		INVALID_DISCARD_LEVEL = 0x7fff
	};

	typedef std::vector<LLFace*> ll_face_list_t;
	typedef std::vector<LLVOVolume*> ll_volume_list_t;

protected:
	/*virtual*/ ~LLViewerImage();
	
public:
	LLViewerImage(const std::string& url, const LLUUID& id, BOOL usemipmaps = TRUE);
	LLViewerImage(const LLUUID& id, const LLHost& host = LLHost::invalid, BOOL usemipmaps = TRUE);
	LLViewerImage(const U32 width, const U32 height, const U8 components, BOOL usemipmaps);
	LLViewerImage(const LLImageRaw* raw, BOOL usemipmaps);

	/*virtual*/ void dump();	// debug info to llinfos

	/*virtual*/ bool bindError(const S32 stage = 0) const;
	/*virtual*/ bool bindDefaultImage(const S32 stage = 0) ;
	/*virtual*/ void forceImmediateUpdate() ;
	
	void reinit(BOOL usemipmaps = TRUE);

	const LLUUID& getID() const { return mID; }

	// New methods for determining image quality/priority
	// texel_area_ratio is ("scaled" texel area)/(original texel area), approximately.
	void addTextureStats(F32 virtual_size, BOOL needs_gltexture = TRUE) const;
	void resetTextureStats();
	void setAdditionalDecodePriority(F32 priority) ;
	F32  maxAdditionalDecodePriority() ;

	BOOL isLargeImage() ;
	BOOL isUpdateFrozen() ;

	// Process image stats to determine priority/quality requirements.
	void processTextureStats();

	// Set callbacks to get called when the image gets updated with higher 
	// resolution versions.
	void setLoadedCallback(loaded_callback_func cb,
						   S32 discard_level, BOOL keep_imageraw, BOOL needs_aux,
						   void* userdata);

	 // ONLY call from LLViewerImageList
	BOOL createTexture(S32 usename = 0);
	void destroyTexture() ;
	void addToCreateTexture();

	BOOL needsAux() const							{ return mNeedsAux; }

	// setDesiredDiscardLevel is only used by LLViewerImageList
	void setDesiredDiscardLevel(S32 discard) { mDesiredDiscardLevel = discard; }
	S32  getDesiredDiscardLevel()			 { return mDesiredDiscardLevel; }

	void setMinDiscardLevel(S32 discard) 	{ mMinDesiredDiscardLevel = llmin(mMinDesiredDiscardLevel,(S8)discard); }
	
	// Host we think might have this image, used for baked av textures.
	LLHost getTargetHost() const			{ return mTargetHost; }

	void setBoostLevel(S32 level);
	S32  getBoostLevel() { return mBoostLevel; }
	
	void updateVirtualSize() ;
	F32 getDecodePriority() const { return mDecodePriority; };
	F32 calcDecodePriority();
	static F32 maxDecodePriority();
	
	// Set the decode priority for this image...
	// DON'T CALL THIS UNLESS YOU KNOW WHAT YOU'RE DOING, it can mess up
	// the priority list, and cause horrible things to happen.
	void setDecodePriority(F32 priority = -1.0f);

	bool updateFetch();
	BOOL hasFetcher() const { return mHasFetcher;}
	// Override the computation of discard levels if we know the exact output
	// size of the image.  Used for UI textures to not decode, even if we have
	// more data.
	void setKnownDrawSize(S32 width, S32 height);

	void setIsMissingAsset();
	BOOL isMissingAsset()	const		{ return mIsMissingAsset; }

	bool hasCallbacks() { return mLoadedCallbackList.empty() ? false : true; }
	
	bool doLoadedCallbacks();

	// returns dimensions of original image for local files (before power of two scaling)
	// and returns 0 for all asset system images
	S32 getOriginalWidth() { return mOrigWidth; }
	S32 getOriginalHeight() { return mOrigHeight; }

	BOOL isForSculptOnly() const ;
	void setForSculpt();
	
	void        checkCachedRawSculptImage() ;
	LLImageRaw* getRawImage()const { return mRawImage ;}
	S32         getRawImageLevel() const {return mRawDiscardLevel;}
	LLImageRaw* getCachedRawImage() const { return mCachedRawImage ;}
	S32         getCachedRawImageLevel() const {return mCachedRawDiscardLevel;}
	BOOL        isCachedRawImageReady() const {return mCachedRawImageReady ;}
	BOOL        isRawImageValid()const { return mIsRawImageValid ; }
	
	void        forceToSaveRawImage(S32 desired_discard = 0) ;
	void        destroySavedRawImage() ;

	BOOL        isSameTexture(const LLViewerImage* tex) const ;

	virtual void addFace(LLFace* facep) ;
	virtual void removeFace(LLFace* facep) ;
	S32 getNumFaces() const;
	const ll_face_list_t* getFaceList() const {return &mFaceList;}
	void reorganizeFaceList() ;

	virtual void addVolume(LLVOVolume* volumep);
	virtual void removeVolume(LLVOVolume* volumep);
	S32 getNumVolumes() const;
	const ll_volume_list_t* getVolumeList() const { return &mVolumeList; }
	void reorganizeVolumeList() ;

	void        setCanUseHTTP(bool can_use_http) {mCanUseHTTP = can_use_http;};

	friend class LocalBitmap; // tag: vaa emerald local_asset_browser

private:
	/*virtual*/ void cleanup(); // Cleanup the LLViewerImage (so we can reinitialize it)

	void init(bool firstinit);

	// Used to be in LLImageGL
	LLImageRaw* readBackRawImage(S8 discard_level = 0);
	void destroyRawImage();
	void saveRawImage() ;
	BOOL forceFetch() ;

	void scaleDown() ;	
	void switchToCachedImage();
	void setCachedRawImage() ;
public:
	S32 mFullWidth;
	S32 mFullHeight;

	S32 mOrigWidth;
	S32 mOrigHeight;
	std::string mUrl;

	// Data used for calculating required image priority/quality level/decimation
	mutable F32 mMaxVirtualSize;	// The largest virtual size of the image, in pixels - how much data to we need?

	F32 mTexelsPerImage;			// Texels per image.
	F32 mDiscardVirtualSize;		// Virtual size used to calculate desired discard
	
	S8  mInImageList;				// TRUE if image is in list (in which case don't reset priority!)
	S8  mIsMediaTexture;			// TRUE if image is being replaced by media (in which case don't update)

	// Various info regarding image requests
	S32 mRequestedDiscardLevel;
	F32 mRequestedDownloadPriority;
	S32 mFetchState;
	U32 mFetchPriority;
	F32 mDownloadProgress;
	F32 mFetchDeltaTime;
	F32 mRequestDeltaTime;
	S32 mDecodeFrame;
	S32 mVisibleFrame; // decode frame where image was last visible
	
	// Timers
	LLFrameTimer mLastPacketTimer;		// Time since last packet.
	LLFrameTimer mLastReferencedTimer;

private:
	LLUUID mID;

	S8  mDesiredDiscardLevel;			// The discard level we'd LIKE to have - if we have it and there's space
	S8  mMinDesiredDiscardLevel;		// The minimum discard level we'd like to have
	S8  mNeedsCreateTexture;	
	mutable S8  mNeedsGLTexture;
	S8  mNeedsAux;					// We need to decode the auxiliary channels
	S8  mDecodingAux;				// Are we decoding high components
	S8  mIsRawImageValid;
	S8  mHasFetcher;				// We've made a fecth request
	S8  mIsFetching;				// Fetch request is active
	S8  mFullyLoaded;
	mutable S8 mIsMissingAsset;		// True if we know that there is no image asset with this image id in the database.	

	// Override the computation of discard levels if we know the exact output size of the image.
	// Used for UI textures to not decode, even if we have more data.
	S32 mKnownDrawWidth;
	S32	mKnownDrawHeight;

	F32 mDecodePriority;			// The priority for decoding this image.
	mutable F32 mAdditionalDecodePriority;  // priority add to mDecodePriority.
	S32 mBoostLevel;				// enum describing priority level
	
	typedef std::list<LLLoadedCallbackEntry*> callback_list_t;
	callback_list_t mLoadedCallbackList;

	LLPointer<LLImageRaw> mRawImage;
	S32 mRawDiscardLevel;
	S32	mMinDiscardLevel;
	F32 mCalculatedDiscardLevel; // Last calculated discard level
	
	//keep a copy of mRawImage for some special purposes
	//when mForceToSaveRawImage is set.
	BOOL mForceToSaveRawImage ;
	LLPointer<LLImageRaw> mSavedRawImage;
	S32 mSavedRawDiscardLevel;
	S32 mDesiredSavedRawDiscardLevel;

	// Used ONLY for cloth meshes right now.  Make SURE you know what you're 
	// doing if you use it for anything else! - djs
	LLPointer<LLImageRaw> mAuxRawImage;

	//a small version of the copy of the raw image (<= 64 * 64)
	LLPointer<LLImageRaw> mCachedRawImage;
	S32 mCachedRawDiscardLevel;
	BOOL mCachedRawImageReady; //the rez of the mCachedRawImage reaches the upper limit.
	
	LLHost mTargetHost;	// if LLHost::invalid, just request from agent's simulator
	
	BOOL   mForSculpt ; //a flag if the texture is used for a sculpt data.
	mutable BOOL    mNeedsResetMaxVirtualSize ;

	ll_face_list_t    mFaceList ; //reverse pointer pointing to the faces using this image as texture
	U32               mNumFaces ;
	LLFrameTimer      mLastFaceListUpdateTimer ;

	ll_volume_list_t  mVolumeList;
	U32				  mNumVolumes;
	LLFrameTimer	  mLastVolumeListUpdateTimer;

	bool mCanUseHTTP; // can this image be fetched by http

public:
	static const U32 sCurrentFileVersion;
	// Default textures
	static LLPointer<LLViewerImage> sMissingAssetImagep;	// Texture to show for an image asset that is not in the database
	static LLPointer<LLViewerImage> sWhiteImagep;	// Texture to show NOTHING (whiteness)
	static LLPointer<LLImageGL> sDefaultImagep; // "Default" texture for error cases
	static LLPointer<LLViewerImage> sSmokeImagep; // Old "Default" translucent texture
	static LLPointer<LLImageGL> sNullImagep; // Null texture for non-textured objects.

	static S32 sImageCount;
	static S32 sRawCount;
	static S32 sAuxCount;
	static LLTimer sEvaluationTimer;
	static S8  sCameraMovingDiscardBias;
	static F32 sDesiredDiscardBias;
	static F32 sDesiredDiscardScale;
	static S32 sBoundTextureMemoryInBytes;
	static S32 sTotalTextureMemoryInBytes;
	static S32 sMaxBoundTextureMemInMegaBytes;
	static S32 sMaxTotalTextureMemInMegaBytes;
	static S32 sMaxDesiredTextureMemInBytes ;
	static BOOL sDontLoadVolumeTextures;

	static S32 sMaxSculptRez ;
	static S32 sMinLargeImageSize ;
	static S32 sMaxSmallImageSize ;
	static BOOL sFreezeImageScalingDown ;//do not scale down image res if set.
	static S32 sLLViewerImageCount ;
};

#endif
