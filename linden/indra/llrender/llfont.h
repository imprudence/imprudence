/** 
 * @file llfont.h
 * @brief Font library wrapper
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc.
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

#ifndef LL_LLFONT_H
#define LL_LLFONT_H

#include <map>
//#include "lllocalidhashmap.h"
#include "llmemory.h"
#include "llstl.h"

#include "llimagegl.h"
#include "llfontbitmapcache.h"

class LLImageRaw;
class LLFontManager;
class LLFont;

// Hack.  FT_Face is just a typedef for a pointer to a struct,
// but there's no simple forward declarations file for FreeType, 
// and the main include file is 200K.  
// We'll forward declare the struct here.  JC
struct FT_FaceRec_;
typedef struct FT_FaceRec_* LLFT_Face;

extern LLFontManager *gFontManagerp;

class LLFontManager
{
public:
	static void initClass();
	static void cleanupClass();
	
public:
	LLFontManager();
	virtual ~LLFontManager();
};

class LLFontGlyphInfo
{
public:
	LLFontGlyphInfo(U32 index);
public:
	U32 mGlyphIndex;
	// Metrics
	S32 mWidth;			// In pixels
	S32 mHeight;		// In pixels
	F32 mXAdvance;		// In pixels
	F32 mYAdvance;		// In pixels
	BOOL mMetricsValid; // We have up-to-date metrics for this glyph

	// Information for actually rendering
	BOOL mIsRendered;	// We actually have rendered this glyph
	S32 mXBitmapOffset; // Offset to the origin in the bitmap
	S32 mYBitmapOffset; // Offset to the origin in the bitmap
	S32 mXBearing;	// Distance from baseline to left in pixels
	S32 mYBearing;	// Distance from baseline to top in pixels
	S32 mBitmapNum; // Which bitmap in the bitmap cache contains this glyph
};

// Used for lists of fallback fonts
class LLFontList : public std::vector<LLFont*>
{
public:
	LLFontList();
	~LLFontList();
	void addAtEnd(LLFont *font);
};

class LLFont
{
public:
	LLFont();
	virtual ~LLFont();

	// is_fallback should be true for fallback fonts that aren't used
	// to render directly (Unicode backup, primarily)
	virtual BOOL loadFace(const std::string& filename,
							const F32 point_size,
							const F32 vert_dpi,
							const F32 horz_dpi,
							const S32 components,
							BOOL is_fallback);
	void setFallbackFont(LLFontList *fontp)		{ mFallbackFontp = fontp; }

	void setCharToGlyphMap(llwchar wch, U32 glyph_index) const;

	// Global font metrics - in units of pixels
	virtual F32 getLineHeight() const;
	virtual F32 getAscenderHeight() const;
	virtual F32 getDescenderHeight() const;


// For a lowercase "g":
//
//	------------------------------
//	                     ^     ^
//						 |     |
//				xxx x    |Ascender
//	           x   x     v     |
//	---------   xxxx-------------- Baseline
//	^		       x	       |
//  | Descender    x           |
//	v			xxxx           |LineHeight
//  -----------------------    |
//                             v
//	------------------------------

	enum
	{
		FIRST_CHAR = 32, 
		NUM_CHARS = 127 - 32, 
		LAST_CHAR_BASIC = 127,

		// Need full 8-bit ascii range for spanish
		NUM_CHARS_FULL = 255 - 32,
		LAST_CHAR_FULL = 255
	};

	const LLFontGlyphInfo &getMetrics(const llwchar wc) const;
	F32 getXAdvance(const llwchar wc) const;
	F32 getXKerning(const llwchar char_left, const llwchar char_right) const; // Get the kerning between the two characters
	virtual void reset() = 0;

protected:
	virtual BOOL hasGlyph(const llwchar wch) const;		// Has a glyph for this character
	virtual BOOL addChar(const llwchar wch) const;		// Add a new character to the font if necessary
	virtual BOOL addGlyph(const llwchar wch, const U32 glyph_index) const;	// Add a new glyph to the existing font
	virtual BOOL addGlyphFromFont(const LLFont *fontp, const llwchar wch, const U32 glyph_index) const;	// Add a glyph from this font to the other (returns the glyph_index, 0 if not found)

	virtual LLFontGlyphInfo* getGlyphInfo(const llwchar wch) const;

	void insertGlyphInfo(llwchar wch, LLFontGlyphInfo* gi) const;
	void renderGlyph(const U32 glyph_index) const;

	void resetBitmapCache();

protected:
	std::string mName;
	F32 mPointSize;
	F32 mAscender;			
	F32 mDescender;
	F32 mLineHeight;

	mutable LLPointer<LLFontBitmapCache> mFontBitmapCachep;

	LLFT_Face mFTFace;

	BOOL mIsFallback;
	LLFontList *mFallbackFontp; // A list of fallback fonts to look for glyphs in (for Unicode chars)

	typedef std::map<llwchar, LLFontGlyphInfo*> char_glyph_info_map_t;
	mutable char_glyph_info_map_t mCharGlyphInfoMap; // Information about glyph location in bitmap

	BOOL mValid;
	void setSubImageLuminanceAlpha(const U32 x,
								   const U32 y,
								   const U32 bitmap_num,
								   const U32 width,
								   const U32 height,
								   const U8 *data,
								   S32 stride = 0) const;
	mutable S32 mRenderGlyphCount;
	mutable S32 mAddGlyphCount;
};

#endif // LL_FONT_
