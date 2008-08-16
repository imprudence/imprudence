/** 
 * @file lluixmltags.h
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#define LL_BUTTON_TAG LLString("button")
#define LL_UI_CTRL_LOCATE_TAG LLString("locate")
#define LL_PAD_TAG LLString("pad")
#define LL_CHECK_BOX_CTRL_TAG LLString("check_box")
#define LL_COMBO_BOX_TAG LLString("combo_box")
#define LL_DRAG_HANDLE_TOP_TAG LLString("drag_handle_top")
#define LL_DRAG_HANDLE_LEFT_TAG LLString("drag_handle_left")
#define LL_FLOATER_TAG LLString("floater")
#define LL_FLOATER_VIEW_TAG LLString("floater_view")
#define LL_MULTI_FLOATER_TAG LLString("multi_floater")
#define LL_ICON_CTRL_TAG LLString("icon")
#define LL_LINE_EDITOR_TAG LLString("line_editor")
#define LL_SEARCH_EDITOR_TAG LLString("search_editor")
#define LL_MENU_ITEM_TAG LLString("menu_item")
#define LL_MENU_GL_TAG LLString("menu")
#define LL_MENU_BAR_GL_TAG LLString("menu_bar")
#define LL_MENU_HOLDER_GL_TAG LLString("menu_holder")
#define LL_PANEL_TAG LLString("panel")
#define LL_RADIO_GROUP_TAG LLString("radio_group")
#define LL_RESIZE_BAR_TAG LLString("resize_bar")
#define LL_RESIZE_HANDLE_TAG LLString("resize_handle")
#define LL_SCROLLBAR_TAG LLString("scrollbar")
#define LL_SCROLLABLE_CONTAINER_VIEW_TAG LLString("scroll_container")
#define LL_SCROLL_LIST_CTRL_TAG LLString("scroll_list")
#define LL_SLIDER_CTRL_TAG LLString("slider")
#define LL_SLIDER_TAG LLString("slider_bar")
#define LL_SPIN_CTRL_TAG LLString("spinner")
#define LL_TAB_CONTAINER_COMMON_TAG LLString("tab_container")
#define LL_TEXT_BOX_TAG LLString("text")
#define LL_TEXT_EDITOR_TAG LLString("text_editor")
#define LL_VIEW_BORDER_TAG LLString("view_border")
#define LL_COLOR_SWATCH_TAG LLString("color_swatch")
#define LL_INVENTORY_PANEL_TAG LLString("inventory_panel")
#define LL_NAME_EDITOR_TAG LLString("name_editor")
#define LL_NAME_LIST_TAG LLString("name_list")
#define LL_TEXTURE_PICKER_TAG LLString("texture_picker")
#define LL_VOLUME_SLIDER_CTRL_TAG LLString("volume_slider")
#define LL_WEB_BROWSER_CTRL_TAG LLString("web_browser")
#define LL_STAT_VIEW_TAG LLString("stat_view")
#define LL_INVENTORY_PANEL_TAG LLString("inventory_panel")
#define LL_PROGRESS_VIEW_TAG LLString("progress_view")
#define LL_STAT_BAR_TAG LLString("stat_bar")
#define LL_STATUS_BAR_TAG LLString("status_bar")
#define LL_VIEWER_TEXT_EDITOR_TAG LLString("viewer_text_editor")
#define LL_TALK_VIEW_TAG LLString("talk_view")
#define LL_COLOR_SWATCH_CTRL_TAG LLString("color_swatch")
#define LL_GL_TEX_MEM_BAR_TAG LLString("tex_mem_bar")
#define LL_TEXTURE_CTRL_TAG LLString("texture_picker")
#define LL_TEXTURE_VIEW_TAG LLString("texture_view")
#define LL_NAME_LIST_CTRL_TAG LLString("name_list")
#define LL_STAT_GRAPH_TAG LLString("stat_graph")
#define LL_NAME_EDITOR_TAG LLString("name_editor")
#define LL_DROP_TARGET_TAG LLString("drop_target")
#define LL_OVERLAY_BAR_TAG LLString("overlay_bar")
#define LL_NET_MAP_TAG LLString("net_map")
#define LL_HUD_VIEW_TAG LLString("hud_view")
#define LL_MEMORY_VIEW_TAG LLString("memory_view")
#define LL_MEDIA_REMOTE_CTRL_TAG LLString("media_remote")
#define LL_MORPH_VIEW_TAG LLString("morph_view")
#define LL_FRAME_STAT_VIEW_TAG LLString("frame_stat_view")
#define LL_FOLDER_VIEW_TAG LLString("folder_view")
#define LL_SNAPSHOT_LIVE_PREVIEW_TAG LLString("snapshot_preview")
#define LL_HOVER_VIEW_TAG LLString("hover_view")
#define LL_VELOCITY_BAR_TAG LLString("velocity_bar")
#define LL_PERMISSIONS_VIEW_TAG LLString("permissions_view")
#define LL_SCROLLING_PANEL_LIST_TAG LLString("scrolling_panel_list")
#define LL_CONTAINER_VIEW_TAG LLString("container_view")
#define LL_CONSOLE_TAG LLString("console")
#define LL_DEBUG_VIEW_TAG LLString("debug_view")
#define LL_AUDIOSTATUS_TAG LLString("audio_status")
#define LL_FAST_TIMER_VIEW_TAG LLString("fast_timer_view")
#define LL_MENU_ITEM_TEAR_OFF_GL_TAG LLString("tearoff_menu")
#define LL_MENU_ITEM_BLANK_GL_TAG LLString("menu_item_blank")
#define LL_MENU_ITEM_CALL_GL_TAG LLString("menu_item_call")
#define LL_MENU_ITEM_CHECK_GL_TAG LLString("menu_item_check")
#define LL_MENU_ITEM_BRANCH_GL_TAG LLString("menu_item_branch")
#define LL_MENU_ITEM_BRANCH_DOWN_GL_TAG LLString("menu_item_branch_down")
#define LL_PIE_MENU_BRANCH_TAG LLString("pie_menu_branch")
#define LL_PIE_MENU_TAG LLString("pie_menu")
#define LL_MENU_ITEM_SEPARATOR_GL_TAG LLString("menu_item_separator")
#define LL_MENU_ITEM_VERTICAL_SEPARATOR_GL_TAG LLString("menu_item_vertical_separator")
#define LL_ROOT_VIEW_TAG LLString("root_view")
#define LL_FOLDER_VIEW_ITEM_TAG LLString("folder_item")
#define LL_FOLDER_VIEW_FOLDER_TAG LLString("folder")
#define LL_TEXTURE_BAR_TAG LLString("texture_bar")
#define LL_JOYSTICK_SLIDE LLString("joystick_slide")
#define LL_JOYSTICK_TURN LLString("joystick_turn")
#define LL_GROUP_DROP_TARGET_TAG LLString("group_drop_target")
#define LL_LAYOUT_STACK_TAG LLString("layout_stack")
#define LL_FLYOUT_BUTTON_TAG "flyout_button"
#endif
