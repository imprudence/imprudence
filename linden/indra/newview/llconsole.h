/** 
 * @file llconsole.h
 * @brief a simple console-style output device
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#ifndef LL_LLCONSOLE_H
#define LL_LLCONSOLE_H

#include "llfixedbuffer.h"
#include "llview.h"
#include "v4color.h"
#include <deque>

class LLFontGL;

class LLConsole : public LLFixedBuffer, public LLView
{
private:
	F32			mLinePersistTime; // Age at which to stop drawing.
	F32			mFadeTime; // Age at which to start fading
	std::deque<LLColor4>	mColors;
	LLFontGL*	mFont;
	S32			mLastBoxHeight;
	S32			mLastBoxWidth;

	struct LineInfo
	{
		LineInfo(const LLWString &wln, F32 sz, const LLColor4& clr, F32 time)
			: wline(wln), size(sz), color(clr), add_time(time)
		{
			
		}
		LLWString wline;
		F32 size;
		LLColor4 color;
		F32 add_time;
	};
	typedef std::list<LineInfo> line_queue_t;
	line_queue_t mLineQueue;
	
public:
	// Font size:
	// -1 = monospace, 0 means small, font size = 1 means big
	LLConsole(const std::string& name, const U32 max_lines, const LLRect &rect, 
		S32 font_size_index, F32 persist_time );
	virtual ~LLConsole();

	virtual EWidgetType getWidgetType() const;
	virtual LLString getWidgetTag() const;

	// each line lasts this long after being added
	void			setLinePersistTime(F32 seconds);

	void			reshape(S32 width, S32 height, BOOL called_from_parent = TRUE);

	// -1 = monospace, 0 means small, font size = 1 means big
	void			setFontSize(S32 size_index);

	void			addLine(const LLString& utf8line, F32 size, const LLColor4 &color);
	void			addLine(const LLWString& wline, F32 size, const LLColor4 &color);
	void			addQueuedLines();
	
	// Overrides
	/*virtual*/ void	draw();
	/*virtual*/ void	addLine(const LLString& utf8line);
	/*virtual*/ void	addLine(const LLWString& line);
	/*virtual*/ void	removeExtraLines();
};

extern LLConsole* gConsole;

#endif
