/** 
 * @file llfloaterassetbrowser.cpp
 * @brief LLFloaterAssetBrowser class implementation
 * Phox wuz hurr
 */

#include "llviewerprecompiledheaders.h"
#include "llfloaterassetbrowser.h"
#include "llinventoryview.h"
#include "llinventorymodel.h"
#include "llviewerimagelist.h"
#include "llbutton.h"
#include "lltextbox.h"
#include "llpreview.h"
#include "llinventorybridge.h"
#include "llagent.h"
#include "lltooldraganddrop.h"
#include "llfocusmgr.h"
#include "llview.h"

// Externs
extern LLInventoryModel gInventory;

// Statics
LLFloaterAssetBrowser* LLFloaterAssetBrowser::sInstance = NULL;

LLFloaterAssetBrowser::LLFloaterAssetBrowser()
:   LLFloater("floater_asset_browser")
{
    LLUICtrlFactory::getInstance()->buildFloater(this, "floater_asset_browser.xml");

	mInventoryPanel = getChild<LLInventoryPanel>("inventory panel");

	//Open all folders and close them in order to make item list up-to-date
	//if(gInventory.getItemCount()==0)
		mInventoryPanel->openAllFolders();

	//gInventory.startBackgroundFetch(gInventory.findCategoryUUIDForType(LLAssetType::AT_TEXTURE));
	
	
	childSetAction("button next", onClickNext, this);
	childSetAction("button previous", onClickPrevious, this);
	childSetAction("button refresh", onClickRefresh, this);
	
	initialize();
	createThumbnails();
	mInventoryPanel->closeAllFolders();
}

// static
void LLFloaterAssetBrowser::show(void*)
{
    if (!sInstance)
	sInstance = new LLFloaterAssetBrowser();

	sInstance->open();
}

// virtual
LLFloaterAssetBrowser::~LLFloaterAssetBrowser()
{
	clearAssetInfoTexts();
	mTextureAssets.clear();
	mMaxIndex = 0;
	mFirstIndex = 0;
	mMouseOverIndex = 0;
	mMouseOverUUID = LLUUID::null;
	mMouseOverAssetUUID = LLUUID::null;
	mFloaterTitle = "";
	clearNoAssetsText();
	sInstance=NULL;
}

void LLFloaterAssetBrowser::initialize()
{
	mMaxIndex = ITEMS_PER_PAGE;
	mFirstIndex = 0;
	mAssetInfoLabelList.clear();
	mAssetInfoIndex = 0;
	mFloaterHeight = getRect().getHeight();
	mFloaterWidth = getRect().getWidth();
	mMouseOverIndex = 0;
	mMouseOverUUID = LLUUID::null;
	mMouseOverAssetUUID = LLUUID::null;
	mFloaterTitle = "";

	S32 image_top = getRect().getHeight();
	S32 image_bottom = BTN_HEIGHT_SMALL;
	S32 image_middle = (image_top + image_bottom) / 2;
	S32 line_height = llround(LLFontGL::getFontSansSerifSmall()->getLineHeight());
	
	mNoAssetsLabel = new LLTextBox("mNoAssetsLabel.", 
										LLRect(getRect().getWidth()/2,
												image_middle + line_height / 2,
												getRect().getWidth(),
												image_middle - line_height / 2 ),
										"No assets found.",
										LLFontGL::getFontSansSerifSmall() );
	mNoAssetsLabel->setFollowsAll();
	addChild(mNoAssetsLabel);
	mNoAssetsLabel->setVisible(FALSE);
}

void LLFloaterAssetBrowser::createThumbnails()
{
	LLViewerInventoryCategory::cat_array_t cats;
	LLViewerInventoryItem::item_array_t items;
	LLIsType isType(LLAssetType::AT_TEXTURE);

	gInventory.collectDescendentsIf(LLUUID::null,
							cats,
							items,
							LLInventoryModel::INCLUDE_TRASH,
							isType);
	
	//Get UUID, asset UUID and name
	for(S32 i = 0; i < items.count(); i++)
	{
		LLInventoryItem* itemp = items[i];
		LLAssetSelection temp;
		temp.mAssetUUID = itemp->getAssetUUID();
		temp.mUUID = itemp->getUUID();
		temp.mName = itemp->getName();
		temp.mTexturep = NULL;
		temp.mAssetRect = LLRect::null;
		mTextureAssets.push_back(temp);
	}
	
	//Get Texturep
	for(S32 i = 0; i < items.count(); i++)
	{
			mTextureAssets[i].mTexturep = gImageList.getImage(mTextureAssets[i].mAssetUUID, MIPMAP_YES, IMMEDIATE_NO);
			mTextureAssets[i].mTexturep->setBoostLevel(LLViewerImage::BOOST_PREVIEW);
			//mTextureAssets[i].mTexturep->processTextureStats();
	}

	//Generate the asset info text
	/*for(S32 i = 0; i < items.count(); i++)
	{
		LLString asset_info;
		LLString dimensions;
		
		asset_info.append(mTextureAssets[i].mName);
		
		//if(mTextureAssets[i].mTexturep->mFullWidth == 0
			//|| mTextureAssets[i].mTexturep->mFullHeight == 0)

		dimensions = llformat("\n%d x %d",
						mTextureAssets[i].mTexturep->mFullWidth,
						mTextureAssets[i].mTexturep->mFullHeight);
		asset_info.append(dimensions);
		
		mTextureAssets[i].mAssetInfo = asset_info;
	}*/
	
	mFloaterTitle = llformat("Asset Browser (%d assets fetched)", mTextureAssets.size());
	setTitle(mFloaterTitle);
}

// virtual
BOOL LLFloaterAssetBrowser::handleHover(S32 x, S32 y, MASK mask)
{
	if(mTextureAssets.size() > 0)
	{
		for(U32 i = mFirstIndex; i <  mMaxIndex; i++)
		{
			if(i < mTextureAssets.size())
			{
    			if(mTextureAssets[i].mAssetRect.pointInRect(x,y))
			    {
					mMouseOverUUID = mTextureAssets[i].mUUID;
				    mMouseOverIndex = i;

				    if(hasMouseCapture())
				    {
    					S32 screen_x;
					    S32 screen_y;
					    LLUUID mObjectUUID= LLUUID::null;
    						const LLViewerInventoryItem *item = gInventory.getItem(mMouseOverUUID);
    
					    localPointToScreen(x, y, &screen_x, &screen_y );
    
					    if(item && item->getPermissions().allowCopyBy(gAgent.getID(), gAgent.getGroupID())
    						&& LLToolDragAndDrop::getInstance()->isOverThreshold(screen_x, screen_y))
					    {
    						EDragAndDropType type;
						    type = LLAssetType::lookupDragAndDropType(item->getType());
						    LLToolDragAndDrop::ESource src = LLToolDragAndDrop::SOURCE_LIBRARY;
    						
						    if(!mObjectUUID.isNull())
						    {
    							src = LLToolDragAndDrop::SOURCE_WORLD;
						    }
						    else if(item->getPermissions().getOwner() == gAgent.getID())
						    {
    							src = LLToolDragAndDrop::SOURCE_AGENT;
						    }
						    LLToolDragAndDrop::getInstance()->beginDrag(type,
													    item->getUUID(),
													    src,
													    mObjectUUID);
						    return LLToolDragAndDrop::getInstance()->handleHover(x, y, mask);
					    }
					    return TRUE;
				    }
				    return LLFloater::handleHover(x, y, mask);
			    }
		    }
		}
		return LLFloater::handleHover(x, y, mask);
	}
	else
	{
		mMouseOverUUID = LLUUID::null;
		return LLFloater::handleHover(x, y, mask);
	}
}

// virtual
BOOL LLFloaterAssetBrowser::handleMouseDown(S32 x, S32 y, MASK mask)
{
	if(mTextureAssets.size() > 0)
	{
		if(mTextureAssets[mMouseOverIndex].mAssetRect.pointInRect(x,y))
		{
			if(mMouseOverUUID.notNull())
			{
				gFocusMgr.setMouseCapture(this);
				S32 screen_x;
				S32 screen_y;
				localPointToScreen(x, y, &screen_x, &screen_y);
				LLToolDragAndDrop::getInstance()->setDragStart(screen_x, screen_y);
				return TRUE;
			}
		}
	}
	return LLFloater::handleMouseDown(x, y, mask);
}

// virtual
BOOL LLFloaterAssetBrowser::handleMouseUp(S32 x, S32 y, MASK mask)
{
	if(hasMouseCapture())
	{
		gFocusMgr.setMouseCapture(NULL);
		return TRUE;
	}
	return LLFloater::handleMouseUp(x, y, mask);
}

// virtual
BOOL LLFloaterAssetBrowser::handleDoubleClick(S32 x, S32 y, MASK mask)
{
	if(mTextureAssets.size() > 0)
	{
		if(mTextureAssets[mMouseOverIndex].mAssetRect.pointInRect(x,y))
		{	
			if(mMouseOverUUID.notNull())
			{
				open_texture(mMouseOverUUID, mTextureAssets[mMouseOverIndex].mName, FALSE);
				return TRUE;
			}
		}
	}
	return LLFloater::handleDoubleClick(x, y, mask);
}

// virtual
void LLFloaterAssetBrowser::draw()
{		
	LLFloater::draw();
	
	if(mTextureAssets.size() > 0)
	{
		S32 hor_pos = 0;
		S32 ver_pos = 0;
		U32 items_per_row = 0;
		S32 height = getRect().getHeight()/8;
		S32 width = height;

		for(U32 i = mFirstIndex; i < mMaxIndex; i++)
		{
			if(i < mTextureAssets.size())
			{
				mTexturep = NULL;
				mImageAssetID = mTextureAssets[i].mAssetUUID;
				
				if(mImageAssetID.notNull())
				{
					mTexturep = gImageList.getImage(mImageAssetID, MIPMAP_YES, IMMEDIATE_NO);
					//mTexturep->setBoostLevel(LLViewerImage::BOOST_PREVIEW);
					mTexturep->processTextureStats();
					mTextureAssets[i].mWidth = mTexturep->mFullWidth;
					mTextureAssets[i].mHeight = mTexturep->mFullHeight;
				}

				if(isMinimized())
				{
					return;
				}

				//Border
				LLRect border;
				border.mLeft = START_POS + ver_pos;
				border.mTop = getRect().getHeight() - LLFLOATER_HEADER_SIZE - BORDER_PAD - hor_pos;
				border.mRight = border.mLeft + width;
				border.mBottom = border.mTop - height;
				
				//Save the rect for each thumbnail
				mTextureAssets[i].mAssetRect = border;
				
				//Asset info rect
				LLRect info_rect;
				info_rect.mLeft = border.mLeft;
				info_rect.mTop = border.mBottom - 3;
				info_rect.mRight = border.mLeft + 130;
				info_rect.mBottom = border.mBottom - 2*llround(LLFontGL::getFontSansSerifSmall()->getLineHeight());

				gl_rect_2d(border, LLColor4::black, FALSE);
				
				// Interior
				LLRect interior = border;
				interior.stretch(1); 
				
				//Draw the image
				if(mTexturep)
				{
					if(mTexturep->getComponents() == 4)
					{
						gl_rect_2d_checkerboard(interior);
					}
					
					gl_draw_scaled_image(interior.mLeft, interior.mBottom, interior.getWidth(), interior.getHeight(), mTexturep);
					// Pump the priority
					mTexturep->addTextureStats((F32)(interior.getWidth() * interior.getHeight()));
					
					//Generate the asset info text
					std::string asset_info;
					asset_info.append(mTextureAssets[i].mName);
					std::string dimensions = llformat("\n%d x %d",
													mTexturep->mFullWidth /*mTextureAssets[i].mWidth*/,
													mTexturep->mFullHeight /* mTextureAssets[i].mHeight*/);
					asset_info.append(dimensions);
					
					// Draw material info below the asset
					// LLTextBox object has to be drawn only once, not non-stop like image
					if(mAssetInfoIndex < ITEMS_PER_PAGE && 
						mAssetInfoIndex < mTextureAssets.size() - mFirstIndex)
					{
						mAssetInfoLabel = new LLTextBox("Asset Info"/*mTextureAssets[i].mAssetInfo*/,
														info_rect,
														asset_info,
														LLFontGL::getFontSansSerifSmall());
						mAssetInfoLabel->setFollowsAll();
						mAssetInfoLabelList.push_back(mAssetInfoLabel);
						addChild(mAssetInfoLabelList[mAssetInfoIndex]);
						mAssetInfoLabelList[mAssetInfoIndex]->setVisible(TRUE);
						mAssetInfoIndex++;
					}
					
				}
				else
				{
					// Draw X
					gl_rect_2d(interior, LLColor4::grey, TRUE);
					gl_draw_x(interior, LLColor4::black);
				}
				//Move to the right
				ver_pos += getRect().getWidth()/6;
				items_per_row++;

				//Change the row
				if(items_per_row % 4 == 0)
				{
					ver_pos = 0;
					hor_pos += getRect().getHeight()/4;
				}
			}
		}//for

		//If the size of the floater has changed, clear the asset info texts
		//in order to draw them again into the new position
		if(getRect().getWidth() != mFloaterWidth || getRect().getHeight() != mFloaterHeight)
		{
			clearAssetInfoTexts();
			//Save the size of the current floater
			mFloaterWidth = getRect().getWidth();
			mFloaterHeight = getRect().getHeight();
		}
		if(mMaxIndex >= mTextureAssets.size())
		{
			childDisable("button next");
			childEnable("button previous");
		}
		else if(mFirstIndex <= 0)
		{
			childEnable("button next");
			childDisable("button previous");
		}
		else
		{
			childEnable("button next");
			childEnable("button previous");
		}
	}
	else
	{
		//No assets found
		mNoAssetsLabel->setVisible(TRUE);
		childDisable("button next");
		childDisable("button previous");
	}
}

void LLFloaterAssetBrowser::clearAssetInfoTexts()
{
	for(U32 i = 0; i < mAssetInfoLabelList.size(); i++)
	{
		delete mAssetInfoLabelList[i];
		mAssetInfoLabelList[i] = NULL;
	}
	mAssetInfoLabelList.clear();
	mAssetInfoIndex = 0;
}

void LLFloaterAssetBrowser::clearNoAssetsText()
{
	if(mTextureAssets.size() > 0)
	{
		delete mNoAssetsLabel;
		mNoAssetsLabel = NULL;
	}
}

// static
void LLFloaterAssetBrowser::onClickNext(void *userdata)
{
	LLFloaterAssetBrowser* self = (LLFloaterAssetBrowser*)userdata;
	self->mFirstIndex = self->mMaxIndex;
	self->mMaxIndex = self->mFirstIndex + ITEMS_PER_PAGE;
	self->clearAssetInfoTexts();
}

// static
void LLFloaterAssetBrowser::onClickPrevious(void *userdata)
{
	LLFloaterAssetBrowser* self = (LLFloaterAssetBrowser*)userdata;
	self->mFirstIndex = self->mMaxIndex - (2 * ITEMS_PER_PAGE);
	self->mMaxIndex = self->mMaxIndex - ITEMS_PER_PAGE;
	self->clearAssetInfoTexts();
}

// static
void LLFloaterAssetBrowser::onClickRefresh(void *userdata)
{
	LLFloaterAssetBrowser* self = (LLFloaterAssetBrowser*)userdata;	
	for(U32 i = 0; i < self->mTextureAssets.size(); i++)
	{
		self->mTextureAssets[i].mTexturep = NULL;
	}
	
	self->mTextureAssets.clear();
	self->createThumbnails();
	self->clearNoAssetsText();
	self->clearAssetInfoTexts();
}
