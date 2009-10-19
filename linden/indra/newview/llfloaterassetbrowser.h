/** 
 * @file llfloaterassetbrowser.h
 * @brief LLFloaterAssetBrowser class implementation
 * Phox wuz hurr
 */

#ifndef LL_LLFLOATERASSETBROWSERVIEW_H
#define LL_LLFLOATERASSETBROWSERVIEW_H

#include "llfloater.h"
#include "llinventorymodel.h"
#include "llview.h"
#include "llviewerimage.h"

class LLInventoryPanel;

struct LLAssetSelection
{
	LLUUID						mUUID;
	LLUUID						mAssetUUID; 
	std::string					mName;
	std::string					mAssetInfo;
	S32							mWidth;
	S32							mHeight;
	LLTextBox*					mAssetInfoTextBox;
	LLPointer<LLViewerImage>	mTexturep;
	LLRect						mAssetRect;
};

class LLFloaterAssetBrowser : public LLFloater
{
public:
    LLFloaterAssetBrowser();

    virtual ~LLFloaterAssetBrowser();

    static void show(void*);
	
	virtual BOOL handleHover(S32 x, S32 y, MASK mask);
	virtual BOOL handleMouseDown(S32 x, S32 y, MASK mask);
	virtual BOOL handleMouseUp(S32 x, S32 y, MASK mask);
	virtual BOOL handleDoubleClick(S32 x, S32 y, MASK mask);
	virtual void draw();

	static LLFloaterAssetBrowser* getInstance(){ return sInstance; }

private:
	void initialize();
	void createThumbnails();
	void clearAssetInfoTexts();
	void clearNoAssetsText();

	static void onClickNext(void *userdata);
	static void onClickPrevious(void *userdata);
	static void onClickRefresh(void *userdata);

	static LLFloaterAssetBrowser* sInstance;

protected:
	LLInventoryPanel*				mInventoryPanel;
	LLUUID							mImageAssetID; 
	LLUUID							mMouseOverUUID;
	LLUUID							mMouseOverAssetUUID;
	LLPointer<LLViewerImage>		mTexturep;
	std::vector<LLAssetSelection>	mTextureAssets;
	std::vector<LLTextBox*>			mAssetInfoLabelList;
	U32								mLastIndex;
	U32								mMaxIndex;
	U32								mFirstIndex;
	U32								mAssetInfoIndex;
	S32								mFloaterHeight;
	S32								mFloaterWidth;
	S32								mMouseOverIndex;
	LLTextBox*						mNoAssetsLabel;
	LLTextBox*						mAssetInfoLabel;
	std::string mFloaterTitle;
};

static const S32 HPAD = 4;
static const S32 BORDER_PAD = HPAD;
static const U32 ITEMS_PER_PAGE = 16;
static const S32 START_POS = 250;

#endif
