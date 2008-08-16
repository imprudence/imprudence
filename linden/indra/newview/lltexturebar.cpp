/** 
 * @file lltexturebar.cpp
 * @brief LLTextureBar class implementation
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#include "lltexturebar.h"
#include "llviewerimage.h"
#include "lltexturetable.h"
#include "llfontgl.h"
#include "llui.h"
#include "llassetstorage.h"
#include "llglheaders.h"
#include "llviewerimagelist.h"

#include "lltexlayer.h"

// colors used for highlighting textures
const F32 select_colors[][4] = {	{1.0f, 1.0f, 1.0f, 0.7f}, // default = white
									{1.0f, 1.0f, 0.3f, 1.0f}, // selected = yellow
									{0.3f, 1.0f, 1.0f, 1.0f}, // flagged = cyan
									{1.0f, 0.3f, 1.0f, 1.0f}, // boosted = magenta
									{0.5f, 0.5f, 1.0f, 1.0f}, // blue
									{0.5f, 1.0f, 0.5f, 1.0f}, // green
									{1.0f, 0.5f, 0.5f, 1.0f},  // updated this frame = red
									{1.0f, 0.8f, 0.8f, 1.0f},  // nodiscard = pale red
									{0.8f, 1.0f, 0.8f, 1.0f},  // fullres = pale green
									{0.8f, 0.8f, 0.8f, 0.7f}   // 0 priority = grey
};

LLTextureBar::LLTextureBar(const std::string& name, const LLRect& r)
	: LLView(name, r, FALSE)
{
	mHilite = 0;
}

static LLString title_string1("Tex UUID Area  DDis(Req)  DecodePri(Req)       [decode]          pkts/max");
static LLString title_string2("W x H (Dis) Mem");

static S32 pip_base = 480;
static S32 pip_width = 6;

EWidgetType LLTextureBar::getWidgetType() const
{
	return WIDGET_TYPE_TEXTURE_BAR;
}

LLString LLTextureBar::getWidgetTag() const
{
	return LL_TEXTURE_BAR_TAG;
}


void LLTextureBar::draw()
{
	if (!mImagep)
	{
		return;
	}

	S32 color;
	if (mHilite)
	{
		color = llclamp(mHilite,0,4);
	}
	else if (mImagep->getBoostLevel())
	{
		color = 3;
	}
	else if (mImagep->mDontDiscard)
	{
		color = 7;
	}
	else if (!mImagep->getUseMipMaps())
	{
		color = 8;
	}
	else if (mImagep->getDecodePriority() == 0.0f)
	{
		color = 9;
	}
	else
	{
		color = 0;
	}

	// We need to draw:
	// The texture UUID or name
	// The progress bar for the texture, highlighted if it's being download
	// Various numerical stats.
	char tex_str[256];
	S32 left, right;
	S32 top = 0;
	S32 bottom = top + 6;

	// Get the name or UUID of the image.
	gTextureTable.getName(mImagep->mID);
	
	// Name, pixel_area, requested pixel area, decode priority
	char uuid_str[255];
	mImagep->mID.toString(uuid_str);
	uuid_str[8] = 0;
	sprintf(tex_str, "%s %7.0f %d(%d) %8.2f(%8.2f)",
			uuid_str,
			mImagep->mMaxVirtualSize,
			mImagep->mDesiredDiscardLevel,
			mImagep->mRequestedDiscardLevel,
			mImagep->getDecodePriority(),
			mImagep->mRequestedDownloadPriority);
			

	LLGLSUIDefault gls_ui;
	LLFontGL::sMonospace->renderUTF8(tex_str, 0, 0, mRect.getHeight(),
									 select_colors[color],
									 LLFontGL::LEFT, LLFontGL::TOP);

	LLGLSNoTexture gls_no_texture;
	F32 last_event;
	LLColor4 clr;
	
	// Draw the packet pip
	last_event = mImagep->mLastPacketTimer.getElapsedTimeF32();
	if (last_event < 1.f)
	{
		clr = LLColor4::white; clr.setAlpha(1.f - last_event);
		glColor4fv(clr.mV);
		gl_rect_2d(pip_base, top, pip_base + pip_width, bottom);
	}
	else
	{
		last_event = mImagep->mRequestTime.getElapsedTimeF32() * 1.f/5.f;
		if (last_event < 1.f)
		{
			clr = LLColor4::green; clr.setAlpha(1.f - last_event);
			glColor4fv(clr.mV);
			gl_rect_2d(pip_base, top, pip_base + pip_width, bottom);
		}
	}
	
	// Draw the decode pip
	if (mImagep->getDiscardLevel() < 0)
		last_event = 1.f;
	else
		last_event = mImagep->mLastDecodeTime.getElapsedTimeF32();
	if (last_event < 1.f)
	{
		clr = LLColor4::yellow; clr.setAlpha(1.f - last_event);
		glColor4fv(clr.mV);
		gl_rect_2d(pip_base + pip_width*2, top, pip_base + pip_width*3, bottom);
	}

	// we don't want to show bind/resident pips for textures using the default texture
	if (mImagep->getHasGLTexture())
	{
		BOOL miss = FALSE;
		// Draw the bound pip
		last_event = mImagep->sLastFrameTime - mImagep->mLastBindTime;
		if (last_event < 1.f)
		{
			clr = LLColor4::magenta1; clr.setAlpha(1.f - last_event);
			glColor4fv(clr.mV);
			gl_rect_2d(pip_base + pip_width*4, top, pip_base + pip_width*5, bottom);

			miss = mImagep->getMissed();
		}

		// Draw the miss pip
		if (miss)
		{
			clr = LLColor4::red;
			glColor4fv(clr.mV);
			gl_rect_2d(pip_base + pip_width*6, top, pip_base + pip_width*7, bottom);
		}
	}

	// Draw the progress bar.
	S32 bar_width = 100;
	S32 bar_left = 280;
	left = bar_left;
	right = left + bar_width;

	glColor4f(0.f, 0.f, 0.f, 0.75f);
	gl_rect_2d(left, top, right, bottom);

	F32 data_progress = 0.0f;
	F32 decode_progress = mImagep->getDecodeProgress(&data_progress);
	
	if (data_progress > 0.0f)
	{
		// Decoded bytes
		right = left + llfloor(decode_progress * (F32)bar_width);

		if (left < right)
		{
			if (mImagep->getDiscardLevel() >= 0)
			{
				glColor4f(0.f, 0.f, 1.f, 0.75f);
			}
			else
			{
				glColor4f(0.5f, 0.0f, 0.5f, 0.75f);
			}
			gl_rect_2d(left, top, right, bottom);
		}

		// Downloaded bytes
		left = right;
		right = left + llfloor((data_progress - decode_progress) * (F32)bar_width);

		if (left < right)
		{
			glColor4f(1.f, 0.f, 0.f, 0.75f);
			gl_rect_2d(left, top, right, bottom);
		}
	}

	{
		LLGLSUIDefault gls_ui;
		// draw the packet data
		{
			LLString num_str = llformat("%3d/%3d", mImagep->mLastPacket+1, mImagep->mPackets);
			LLFontGL::sMonospace->renderUTF8(num_str, 0, bar_left + 100, mRect.getHeight(), select_colors[color],
											 LLFontGL::LEFT, LLFontGL::TOP);
		}
		
		// draw the image size at the end
		{
			LLString num_str = llformat("%4d x %4d (%d) %7d", mImagep->getWidth(), mImagep->getHeight(),
										mImagep->getDiscardLevel(), mImagep->mTextureMemory);
			LLFontGL::sMonospace->renderUTF8(num_str, 0, pip_base + pip_width*8, mRect.getHeight(), select_colors[color],
											LLFontGL::LEFT, LLFontGL::TOP);
		}
	}

}

BOOL LLTextureBar::handleMouseDown(S32 x, S32 y, MASK mask)
{
	return FALSE;
}

LLRect LLTextureBar::getRequiredRect()
{
	LLRect rect;

	rect.mTop = 8;

	return rect;
}

//==============================================================================

LLGLTexMemBar::LLGLTexMemBar(const std::string& name)
	: LLView(name, FALSE)
{
	S32 line_height = (S32)(LLFontGL::sMonospace->getLineHeight() + .5f);
	setRect(LLRect(0,0,100,line_height * 4));
	updateRect();
}

EWidgetType LLGLTexMemBar::getWidgetType() const
{
	return WIDGET_TYPE_TEX_MEM_BAR;
}

LLString LLGLTexMemBar::getWidgetTag() const
{
	return LL_GL_TEX_MEM_BAR_TAG;
}


extern F32 texmem_lower_bound_scale;

void LLGLTexMemBar::draw()
{
	S32 bound_mem = LLViewerImage::sBoundTextureMemory;
 	S32 max_bound_mem = LLViewerImage::sMaxBoundTextureMem;
	S32 total_mem = LLViewerImage::sTotalTextureMemory;
	S32 max_total_mem = LLViewerImage::sMaxTotalTextureMem;
	F32 discard_bias = LLViewerImage::sDesiredDiscardBias;
	S32 line_height = (S32)(LLFontGL::sMonospace->getLineHeight() + .5f);
	
	//----------------------------------------------------------------------------
	LLGLSUIDefault gls_ui;
	F32 text_color[] = {1.f, 1.f, 1.f, 0.75f};
	
	std::string text;
	text = llformat("GL Tot: %d/%d MB Bound: %d/%d MB Discard Bias: %.2f",
					total_mem/(1024*1024),
					max_total_mem/(1024*1024),
					bound_mem/(1024*1024),
					max_bound_mem/(1024*1024),
					discard_bias);

	LLFontGL::sMonospace->renderUTF8(text, 0, 0, line_height*3,
									 text_color, LLFontGL::LEFT, LLFontGL::TOP);

	//----------------------------------------------------------------------------
	S32 bar_left = 350;
	S32 bar_width = 200;
	S32 top = line_height*3 - 2;
	S32 bottom = top - 6;
	S32 left = bar_left;
	S32 right = left + bar_width;

	F32 bar_scale = (F32)bar_width / (max_bound_mem * 1.5f);
	
	LLGLSNoTexture gls_no_texture;
	
	glColor4f(0.5f, 0.5f, 0.5f, 0.75f);
	gl_rect_2d(left, top, right, bottom);

	
	left = bar_left;
	right = left + llfloor(bound_mem * bar_scale);
	if (bound_mem < llfloor(max_bound_mem * texmem_lower_bound_scale))
	{
		glColor4f(0.f, 1.f, 0.f, 0.75f);
	}
	else if (bound_mem < max_bound_mem)
	{
		glColor4f(1.f, 1.f, 0.f, 0.75f);
	}
	else
	{
		glColor4f(1.f, 0.f, 0.f, 0.75f);
	}
	gl_rect_2d(left, top, right, bottom);

	bar_scale = (F32)bar_width / (max_total_mem * 1.5f);
	
	top = bottom - 2;
	bottom = top - 6;
	left = bar_left;
	right = left + llfloor(total_mem * bar_scale);
	if (total_mem < llfloor(max_total_mem * texmem_lower_bound_scale))
	{
		glColor4f(0.f, 1.f, 0.f, 0.75f);
	}
	else if (total_mem < max_total_mem)
	{
		glColor4f(1.f, 1.f, 0.f, 0.75f);
	}
	else
	{
		glColor4f(1.f, 0.f, 0.f, 0.75f);
	}
	gl_rect_2d(left, top, right, bottom);
	
	//----------------------------------------------------------------------------
	text = llformat("TexStaticImageList Raw: %d GL: %d TGA: %d",
					gTexStaticImageList.mRawBytes, gTexStaticImageList.mGLBytes, gTexStaticImageList.mTGABytes);

	LLGLEnable tex(GL_TEXTURE_2D);
	LLFontGL::sMonospace->renderUTF8(text, 0, 0, line_height*2,
									 text_color, LLFontGL::LEFT, LLFontGL::TOP);
	

	//----------------------------------------------------------------------------

	S32 x = 0;
	LLFontGL::sMonospace->renderUTF8(title_string1, 0, x, line_height,
									 text_color, LLFontGL::LEFT, LLFontGL::TOP);
	S32 dx = 24;
	x = pip_base - dx;
	LLFontGL::sMonospace->renderUTF8("pkt", 0, x, line_height, LLColor4::white, LLFontGL::LEFT, LLFontGL::TOP);
	x += dx;
	LLFontGL::sMonospace->renderUTF8("dec", 0, x, line_height, LLColor4::yellow, LLFontGL::LEFT, LLFontGL::TOP);
	x += dx;
	LLFontGL::sMonospace->renderUTF8("bnd", 0, x, line_height, LLColor4::magenta1, LLFontGL::LEFT, LLFontGL::TOP);
	x += dx;
	LLFontGL::sMonospace->renderUTF8("mis", 0, x, line_height, LLColor4::red, LLFontGL::LEFT, LLFontGL::TOP);
	x += dx;
	LLFontGL::sMonospace->renderUTF8(title_string2, 0, x, line_height,
									 text_color, LLFontGL::LEFT, LLFontGL::TOP);
}

BOOL LLGLTexMemBar::handleMouseDown(S32 x, S32 y, MASK mask)
{
	return FALSE;
}

LLRect LLGLTexMemBar::getRequiredRect()
{
	LLRect rect;
	rect.mTop = 8;
	return rect;
}

