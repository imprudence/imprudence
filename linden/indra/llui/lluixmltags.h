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

char const* const LL_BUTTON_TAG = "button";
char const* const LL_UI_CTRL_LOCATE_TAG = "locate";
char const* const LL_PAD_TAG = "pad";
char const* const LL_CHECK_BOX_CTRL_TAG = "check_box";
char const* const LL_COMBO_BOX_TAG = "combo_box";
char const* const LL_DRAG_HANDLE_TOP_TAG = "drag_handle_top";
char const* const LL_DRAG_HANDLE_LEFT_TAG = "drag_handle_left";
char const* const LL_FLOATER_TAG = "floater";
char const* const LL_FLOATER_VIEW_TAG = "floater_view";
char const* const LL_MULTI_FLOATER_TAG = "multi_floater";
char const* const LL_ICON_CTRL_TAG = "icon";
char const* const LL_LINE_EDITOR_TAG = "line_editor";
char const* const LL_SEARCH_EDITOR_TAG = "search_editor";
char const* const LL_MENU_ITEM_TAG = "menu_item";
char const* const LL_MENU_GL_TAG = "menu";
char const* const LL_MENU_BAR_GL_TAG = "menu_bar";
char const* const LL_MENU_HOLDER_GL_TAG = "menu_holder";
char const* const LL_PANEL_TAG = "panel";
char const* const LL_RADIO_GROUP_TAG = "radio_group";
char const* const LL_RESIZE_BAR_TAG = "resize_bar";
char const* const LL_RESIZE_HANDLE_TAG = "resize_handle";
char const* const LL_SCROLLBAR_TAG = "scrollbar";
char const* const LL_SCROLLABLE_CONTAINER_VIEW_TAG = "scroll_container";
char const* const LL_SCROLL_LIST_CTRL_TAG = "scroll_list";
char const* const LL_SLIDER_CTRL_TAG = "slider";
char const* const LL_SLIDER_TAG = "slider_bar";
char const* const LL_MULTI_SLIDER_CTRL_TAG = "multi_slider";
char const* const LL_MULTI_SLIDER_TAG = "multi_slider_bar";
char const* const LL_SPIN_CTRL_TAG = "spinner";
char const* const LL_TAB_CONTAINER_COMMON_TAG = "tab_container";
char const* const LL_TEXT_BOX_TAG = "text";
char const* const LL_TEXT_EDITOR_TAG = "text_editor";
char const* const LL_VIEW_BORDER_TAG = "view_border";
char const* const LL_COLOR_SWATCH_TAG = "color_swatch";
char const* const LL_INVENTORY_PANEL_TAG = "inventory_panel";
char const* const LL_NAME_EDITOR_TAG = "name_editor";
char const* const LL_NAME_LIST_TAG = "name_list";
char const* const LL_TEXTURE_PICKER_TAG = "texture_picker";
char const* const LL_VOLUME_SLIDER_CTRL_TAG = "volume_slider";
char const* const LL_WEB_BROWSER_CTRL_TAG = "web_browser";
char const* const LL_STAT_VIEW_TAG = "stat_view";
char const* const LL_PROGRESS_VIEW_TAG = "progress_view";
char const* const LL_STAT_BAR_TAG = "stat_bar";
char const* const LL_STATUS_BAR_TAG = "status_bar";
char const* const LL_VIEWER_TEXT_EDITOR_TAG = "viewer_text_editor";
char const* const LL_TALK_VIEW_TAG = "talk_view";
char const* const LL_COLOR_SWATCH_CTRL_TAG = "color_swatch";
char const* const LL_GL_TEX_MEM_BAR_TAG = "tex_mem_bar";
char const* const LL_TEXTURE_CTRL_TAG = "texture_picker";
char const* const LL_TEXTURE_VIEW_TAG = "texture_view";
char const* const LL_NAME_LIST_CTRL_TAG = "name_list";
char const* const LL_STAT_GRAPH_TAG = "stat_graph";
char const* const LL_DROP_TARGET_TAG = "drop_target";
char const* const LL_OVERLAY_BAR_TAG = "overlay_bar";
char const* const LL_NET_MAP_TAG = "net_map";
char const* const LL_HUD_VIEW_TAG = "hud_view";
char const* const LL_MEMORY_VIEW_TAG = "memory_view";
char const* const LL_MEDIA_REMOTE_CTRL_TAG = "media_remote";
char const* const LL_MORPH_VIEW_TAG = "morph_view";
char const* const LL_FRAME_STAT_VIEW_TAG = "frame_stat_view";
char const* const LL_FOLDER_VIEW_TAG = "folder_view";
char const* const LL_SNAPSHOT_LIVE_PREVIEW_TAG = "snapshot_preview";
char const* const LL_HOVER_VIEW_TAG = "hover_view";
char const* const LL_VELOCITY_BAR_TAG = "velocity_bar";
char const* const LL_PERMISSIONS_VIEW_TAG = "permissions_view";
char const* const LL_SCROLLING_PANEL_LIST_TAG = "scrolling_panel_list";
char const* const LL_CONTAINER_VIEW_TAG = "container_view";
char const* const LL_CONSOLE_TAG = "console";
char const* const LL_DEBUG_VIEW_TAG = "debug_view";
char const* const LL_AUDIOSTATUS_TAG = "audio_status";
char const* const LL_FAST_TIMER_VIEW_TAG = "fast_timer_view";
char const* const LL_MENU_ITEM_TEAR_OFF_GL_TAG = "tearoff_menu";
char const* const LL_MENU_ITEM_BLANK_GL_TAG = "menu_item_blank";
char const* const LL_MENU_ITEM_CALL_GL_TAG = "menu_item_call";
char const* const LL_MENU_ITEM_CHECK_GL_TAG = "menu_item_check";
char const* const LL_MENU_ITEM_BRANCH_GL_TAG = "menu_item_branch";
char const* const LL_MENU_ITEM_BRANCH_DOWN_GL_TAG = "menu_item_branch_down";
char const* const LL_PIE_MENU_BRANCH_TAG = "pie_menu_branch";
char const* const LL_PIE_MENU_TAG = "pie_menu";
char const* const LL_MENU_ITEM_SEPARATOR_GL_TAG = "menu_item_separator";
char const* const LL_MENU_ITEM_VERTICAL_SEPARATOR_GL_TAG = "menu_item_vertical_separator";
char const* const LL_ROOT_VIEW_TAG = "root_view";
char const* const LL_FOLDER_VIEW_ITEM_TAG = "folder_item";
char const* const LL_FOLDER_VIEW_FOLDER_TAG = "folder";
char const* const LL_TEXTURE_BAR_TAG = "texture_bar";
char const* const LL_JOYSTICK_SLIDE = "joystick_slide";
char const* const LL_JOYSTICK_TURN = "joystick_turn";
char const* const LL_GROUP_DROP_TARGET_TAG = "group_drop_target";
char const* const LL_LAYOUT_STACK_TAG = "layout_stack";
char const* const LL_LAYOUT_PANEL_TAG = "layout_panel";
char const* const LL_FLYOUT_BUTTON_TAG = "flyout_button";
char const* const LL_FLYOUT_BUTTON_ITEM_TAG = "flyout_button_item";
char const* const LL_SIMPLE_TEXT_EDITOR_TAG = "simple_text_editor";
char const* const LL_RADIO_ITEM_TAG = "radio_item";
char const* const LL_PROGRESS_BAR_TAG = "progress_bar";

#endif
