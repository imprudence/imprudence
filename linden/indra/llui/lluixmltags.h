/** 
 * @file lluixmltags.h
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2009, Linden Research, Inc.
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

#ifndef LL_UI_XML_TAGS_H
#define LL_UI_XML_TAGS_H

const std::string
	LL_BUTTON_TAG("button"),
	LL_UI_CTRL_LOCATE_TAG("locate"),
	LL_PAD_TAG("pad"),
	LL_CHECK_BOX_CTRL_TAG("check_box"),
	LL_COMBO_BOX_TAG("combo_box"),
	LL_DRAG_HANDLE_TOP_TAG("drag_handle_top"),
	LL_DRAG_HANDLE_LEFT_TAG("drag_handle_left"),
	LL_FLOATER_TAG("floater"),
	LL_FLOATER_VIEW_TAG("floater_view"),
	LL_MULTI_FLOATER_TAG("multi_floater"),
	LL_ICON_CTRL_TAG("icon"),
	LL_LINE_EDITOR_TAG("line_editor"),
	LL_SEARCH_EDITOR_TAG("search_editor"),
	LL_MENU_ITEM_TAG("menu_item"),
	LL_MENU_GL_TAG("menu"),
	LL_MENU_BAR_GL_TAG("menu_bar"),
	LL_MENU_HOLDER_GL_TAG("menu_holder"),
	LL_PANEL_TAG("panel"),
	LL_RADIO_GROUP_TAG("radio_group"),
	LL_RESIZE_BAR_TAG("resize_bar"),
	LL_RESIZE_HANDLE_TAG("resize_handle"),
	LL_SCROLLBAR_TAG("scrollbar"),
	LL_SCROLLABLE_CONTAINER_VIEW_TAG("scroll_container"),
	LL_SCROLL_LIST_CTRL_TAG("scroll_list"),
	LL_SLIDER_CTRL_TAG("slider"),
	LL_SLIDER_TAG("slider_bar"),
	LL_MULTI_SLIDER_CTRL_TAG("multi_slider"),
	LL_MULTI_SLIDER_TAG("multi_slider_bar"),
	LL_SPIN_CTRL_TAG("spinner"),
	LL_TAB_CONTAINER_COMMON_TAG("tab_container"),
	LL_TEXT_BOX_TAG("text"),
	LL_TEXT_EDITOR_TAG("text_editor"),
	LL_VIEW_BORDER_TAG("view_border"),
	LL_COLOR_SWATCH_TAG("color_swatch"),
	LL_INVENTORY_PANEL_TAG("inventory_panel"),
	LL_NAME_EDITOR_TAG("name_editor"),
	LL_NAME_LIST_TAG("name_list"),
	LL_TEXTURE_PICKER_TAG("texture_picker"),
	LL_VOLUME_SLIDER_CTRL_TAG("volume_slider"),
	LL_WEB_BROWSER_CTRL_TAG("web_browser"),
	LL_STAT_VIEW_TAG("stat_view"),
	LL_PROGRESS_VIEW_TAG("progress_view"),
	LL_STAT_BAR_TAG("stat_bar"),
	LL_STATUS_BAR_TAG("status_bar"),
	LL_VIEWER_TEXT_EDITOR_TAG("viewer_text_editor"),
	LL_TALK_VIEW_TAG("talk_view"),
	LL_COLOR_SWATCH_CTRL_TAG("color_swatch"),
	LL_GL_TEX_MEM_BAR_TAG("tex_mem_bar"),
	LL_TEXTURE_CTRL_TAG("texture_picker"),
	LL_TEXTURE_VIEW_TAG("texture_view"),
	LL_NAME_LIST_CTRL_TAG("name_list"),
	LL_STAT_GRAPH_TAG("stat_graph"),
	LL_DROP_TARGET_TAG("drop_target"),
	LL_OVERLAY_BAR_TAG("overlay_bar"),
	LL_NET_MAP_TAG("net_map"),
	LL_HUD_VIEW_TAG("hud_view"),
	LL_MEMORY_VIEW_TAG("memory_view"),
	LL_MEDIA_REMOTE_CTRL_TAG("media_remote"),
	LL_MORPH_VIEW_TAG("morph_view"),
	LL_FRAME_STAT_VIEW_TAG("frame_stat_view"),
	LL_FOLDER_VIEW_TAG("folder_view"),
	LL_SNAPSHOT_LIVE_PREVIEW_TAG("snapshot_preview"),
	LL_HOVER_VIEW_TAG("hover_view"),
	LL_VELOCITY_BAR_TAG("velocity_bar"),
	LL_PERMISSIONS_VIEW_TAG("permissions_view"),
	LL_SCROLLING_PANEL_LIST_TAG("scrolling_panel_list"),
	LL_CONTAINER_VIEW_TAG("container_view"),
	LL_CONSOLE_TAG("console"),
	LL_DEBUG_VIEW_TAG("debug_view"),
	LL_AUDIOSTATUS_TAG("audio_status"),
	LL_FAST_TIMER_VIEW_TAG("fast_timer_view"),
	LL_MENU_ITEM_TEAR_OFF_GL_TAG("tearoff_menu"),
	LL_MENU_ITEM_BLANK_GL_TAG("menu_item_blank"),
	LL_MENU_ITEM_CALL_GL_TAG("menu_item_call"),
	LL_MENU_ITEM_CHECK_GL_TAG("menu_item_check"),
	LL_MENU_ITEM_BRANCH_GL_TAG("menu_item_branch"),
	LL_MENU_ITEM_BRANCH_DOWN_GL_TAG("menu_item_branch_down"),
	LL_PIE_MENU_BRANCH_TAG("pie_menu_branch"),
	LL_PIE_MENU_TAG("pie_menu"),
	LL_MENU_ITEM_SEPARATOR_GL_TAG("menu_item_separator"),
	LL_MENU_ITEM_VERTICAL_SEPARATOR_GL_TAG("menu_item_vertical_separator"),
	LL_ROOT_VIEW_TAG("root_view"),
	LL_FOLDER_VIEW_ITEM_TAG("folder_item"),
	LL_FOLDER_VIEW_FOLDER_TAG("folder"),
	LL_TEXTURE_BAR_TAG("texture_bar"),
	LL_JOYSTICK_SLIDE("joystick_slide"),
	LL_JOYSTICK_TURN("joystick_turn"),
	LL_GROUP_DROP_TARGET_TAG("group_drop_target"),
	LL_LAYOUT_STACK_TAG("layout_stack"),
	LL_LAYOUT_PANEL_TAG("layout_panel"),
	LL_FLYOUT_BUTTON_TAG("flyout_button"),
	LL_FLYOUT_BUTTON_ITEM_TAG("flyout_button_item"),
	LL_SIMPLE_TEXT_EDITOR_TAG("simple_text_editor"),
	LL_RADIO_ITEM_TAG("radio_item"),
	LL_PROGRESS_BAR_TAG("progress_bar");
#endif
