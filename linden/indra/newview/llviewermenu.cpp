/** 
 * @file llviewermenu.cpp
 * @brief Builds menus out of items.
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

#include "llviewerprecompiledheaders.h"

#include "llviewermenu.h" 

// system library includes
#include <iostream>
#include <fstream>
#include <sstream>

// linden library includes
#include "audioengine.h"
#include "indra_constants.h"
#include "llassetstorage.h"
#include "llchat.h"
#include "llfeaturemanager.h"
#include "llfocusmgr.h"
#include "llfontgl.h"
#include "llinstantmessage.h"
#include "llpermissionsflags.h"
#include "llrect.h"
#include "llsecondlifeurls.h"
#include "lltransactiontypes.h"
#include "llui.h"
#include "llview.h"
#include "llxfermanager.h"
#include "message.h"
#include "raytrace.h"
#include "llsdserialize.h"
#include "lltimer.h"
#include "llvfile.h"
#include "llvolumemgr.h"

// newview includes
#include "llagent.h"

#include "llagentpilot.h"
#include "llbox.h"
#include "llcallingcard.h"
#include "llclipboard.h"
#include "llcompilequeue.h"
#include "llconsole.h"
#include "llviewercontrol.h"
#include "lldebugview.h"
#include "lldir.h"
#include "lldrawable.h"
#include "lldrawpoolalpha.h"
#include "lldrawpooltree.h"
#include "llface.h"
#include "llfirstuse.h"
#include "llfloater.h"
#include "llfloaterabout.h"
#include "llfloaterbuycurrency.h"
#include "llfloateractivespeakers.h"
#include "llfloateranimpreview.h"
#include "llfloateravatarinfo.h"
#include "llfloateravatartextures.h"
#include "llfloaterbeacons.h"
#include "llfloaterbuildoptions.h"
#include "llfloaterbulkpermission.h"
#include "llfloaterbump.h"
#include "llfloaterbuy.h"
#include "llfloaterbuycontents.h"
#include "llfloaterbuycurrency.h"
#include "llfloaterbuyland.h"
#include "llfloatercamera.h"
#include "llfloaterchat.h"
#include "llfloatercustomize.h"
#include "llfloaterdaycycle.h"
#include "llfloaterdirectory.h"
#include "llfloatereditui.h"
#include "llfloaterchatterbox.h"
#include "llfloaterfriends.h"
#include "llfloatergesture.h"
#include "llfloatergodtools.h"
#include "llfloatergroupinfo.h"
#include "llfloatergroupinvite.h"
#include "llfloatergroups.h"
#include "llfloaterhtmlhelp.h"
#include "llfloaterhud.h"
#include "llfloaterinspect.h"
#include "llfloaterlagmeter.h"
#include "llfloaterland.h"
#include "llfloaterlandholdings.h"
#include "llfloatermap.h"
#include "llfloatermute.h"
#include "llfloateropenobject.h"
#include "llfloaterpermissionsmgr.h"
#include "llfloaterpostprocess.h"
#include "llfloaterpreference.h"
#include "llfloaterregioninfo.h"
#include "llfloaterreporter.h"
#include "llfloaterscriptdebug.h"
#include "llfloatersettingsdebug.h"
#include "llfloaterenvsettings.h"
#include "llfloaterstats.h"
#include "llfloatertest.h"
#include "llfloatertools.h"
#include "llfloaterwater.h"
#include "llfloaterwindlight.h"
#include "llfloaterworldmap.h"
#include "llfloatermemleak.h"
#include "llframestats.h"
#include "llframestatview.h"
#include "llfasttimerview.h"
#include "llmemoryview.h"
#include "llgivemoney.h"
#include "llgroupmgr.h"
#include "llhoverview.h"
#include "llhudeffecttrail.h"
#include "llhudmanager.h"
#include "llimage.h"
#include "llimagebmp.h"
#include "llimagej2c.h"
#include "llimagetga.h"
#include "llinventorymodel.h"
#include "llinventoryview.h"
#include "llkeyboard.h"
#include "lllineeditor.h"
#include "llmenucommands.h"
#include "llmenugl.h"
#include "llmorphview.h"
#include "llmoveview.h"
#include "llmutelist.h"
#include "llnotify.h"
#include "llpanellogin.h"
#include "llpanelobject.h"
#include "llparcel.h"
#include "llprimitive.h"
#include "llresmgr.h"
#include "llselectmgr.h"
#include "llsky.h"
#include "llstatusbar.h"
#include "llstatview.h"
#include "llstring.h"
#include "llsurfacepatch.h"
#include "llimview.h"
#include "lltextureview.h"
#include "lltool.h"
#include "lltoolbar.h"
#include "lltoolcomp.h"
#include "lltoolface.h"
#include "lltoolfocus.h"
#include "lltoolgrab.h"
#include "lltoolmgr.h"
#include "lltoolpie.h"
#include "lltoolplacer.h"
#include "lltoolselectland.h"
#include "lltrans.h"
#include "lluictrlfactory.h"
#include "lluploaddialog.h"
#include "lluserauth.h"
#include "lluuid.h"
#include "llvelocitybar.h"
#include "llviewercamera.h"
#include "llviewergenericmessage.h"
#include "llviewergesture.h"
#include "llviewerinventory.h"
#include "llviewermenufile.h"	// init_menu_file()
#include "llviewermessage.h"
#include "llviewernetwork.h"
#include "llviewerobjectlist.h"
#include "llviewerparcelmgr.h"
#include "llviewerparceloverlay.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "llvolume.h"
#include "llweb.h"
#include "llworld.h"
#include "llworldmap.h"
#include "object_flags.h"
#include "pipeline.h"
#include "llappviewer.h"
#include "roles_constants.h"
#include "llviewerjoystick.h"
#include "llwlanimator.h"
#include "llwlparammanager.h"
#include "llwaterparammanager.h"

#include "lltexlayer.h"
#include "primbackup.h"

#include "jcfloater_animation_list.h"
#include "llfloaterassetbrowser.h"

void init_client_menu(LLMenuGL* menu);
void init_server_menu(LLMenuGL* menu);

void init_debug_world_menu(LLMenuGL* menu);
void init_debug_rendering_menu(LLMenuGL* menu);
void init_debug_ui_menu(LLMenuGL* menu);
void init_debug_xui_menu(LLMenuGL* menu);
void init_debug_avatar_menu(LLMenuGL* menu);
void init_debug_baked_texture_menu(LLMenuGL* menu);
// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
#ifdef RLV_DEBUG_TESTS
	#include "rlvtest.h"
#endif // RLV_DEBUG_TESTS
#include "rlvfloaterbehaviour.h"
void init_debug_rlva_menu(LLMenuGL* menu);
// [/RLVa:KB]

BOOL enable_land_build(void*);
BOOL enable_object_build(void*);

LLVOAvatar* find_avatar_from_object( LLViewerObject* object );
LLVOAvatar* find_avatar_from_object( const LLUUID& object_id );

void handle_test_load_url(void*);

//
// Evil hackish imported globals
//
extern BOOL	gRenderLightGlows;
extern BOOL gRenderAvatar;
extern BOOL	gHideSelectedObjects;
extern BOOL gShowOverlayTitle;
extern BOOL gOcclusionCull;
extern BOOL gAllowSelectAvatar;

//
// Globals
//

LLMenuBarGL		*gMenuBarView = NULL;
LLViewerMenuHolderGL	*gMenuHolder = NULL;
LLMenuGL		*gPopupMenuView = NULL;
LLMenuBarGL		*gLoginMenuBarView = NULL;

// Pie menus
LLPieMenu	*gPieSelf	= NULL;
LLPieMenu	*gPieAvatar = NULL;
LLPieMenu	*gPieObject = NULL;
LLPieMenu	*gPieAttachment = NULL;
LLPieMenu	*gPieHUD = NULL;
LLPieMenu	*gPieLand	= NULL;

// local constants
const std::string CLIENT_MENU_NAME("Advanced");
const std::string SERVER_MENU_NAME("Admin");

const std::string SAVE_INTO_INVENTORY("Save Object Back to My Inventory");
const std::string SAVE_INTO_TASK_INVENTORY("Save Object Back to Object Contents");

LLMenuGL* gAttachSubMenu = NULL;
LLMenuGL* gDetachSubMenu = NULL;
LLMenuGL* gTakeOffClothes = NULL;
LLPieMenu* gAttachScreenPieMenu = NULL;
LLPieMenu* gAttachPieMenu = NULL;
LLPieMenu* gAttachBodyPartPieMenus[8];
LLPieMenu* gDetachPieMenu = NULL;
LLPieMenu* gDetachScreenPieMenu = NULL;
LLPieMenu* gDetachBodyPartPieMenus[8];

LLMenuItemCallGL* gAFKMenu = NULL;
LLMenuItemCallGL* gBusyMenu = NULL;

typedef LLMemberListener<LLView> view_listener_t;

//
// Local prototypes
//
void handle_leave_group(void *);

// File Menu
const char* upload_pick(void* data);
void handle_upload(void* data);
//void handle_upload_object(void* data);
void handle_compress_image(void*);
BOOL enable_save_as(void *);

// Edit menu
void handle_dump_group_info(void *);
void handle_dump_capabilities_info(void *);
void handle_dump_focus(void*);

void handle_region_dump_settings(void*);
void handle_region_dump_temp_asset_data(void*);
void handle_region_clear_temp_asset_data(void*);

// Object pie menu
BOOL sitting_on_selection();

void near_sit_object();
void label_sit_or_stand(std::string& label, void*);
// buy and take alias into the same UI positions, so these
// declarations handle this mess.
BOOL enable_take();
void handle_take();
void confirm_take(S32 option, void* data);
BOOL enable_buy(void*); 
void handle_buy(void *);
void handle_buy_object(LLSaleInfo sale_info);
void handle_buy_contents(LLSaleInfo sale_info);
void label_touch(std::string& label, void*);

// Land pie menu
void near_sit_down_point(BOOL success, void *);

// Avatar pie menu
void handle_follow(void *userdata);
void handle_talk_to(void *userdata);

// Debug menu
void show_permissions_control(void*);
void toggle_build_options(void* user_data);
void reload_ui(void*);
void handle_agent_stop_moving(void*);
void print_packets_lost(void*);
void drop_packet(void*);
void velocity_interpolate( void* data );
void update_fov(S32 increments);
void toggle_wind_audio(void);
void toggle_water_audio(void);
void handle_rebake_textures(void*);
BOOL check_admin_override(void*);
void handle_admin_override_toggle(void*);
#ifdef TOGGLE_HACKED_GODLIKE_VIEWER
void handle_toggle_hacked_godmode(void*);
BOOL check_toggle_hacked_godmode(void*);
#endif

void toggle_glow(void *);
BOOL check_glow(void *);

void toggle_vertex_shaders(void *);
BOOL check_vertex_shaders(void *);

void toggle_cull_small(void *);

void toggle_show_xui_names(void *);
BOOL check_show_xui_names(void *);

void run_vectorize_perf_test(void *)
{
	gSavedSettings.setBOOL("VectorizePerfTest", TRUE);
}

// Debug UI
void handle_web_search_demo(void*);
void handle_slurl_test(void*);
void handle_save_to_xml(void*);
void handle_load_from_xml(void*);

void handle_god_mode(void*);

// God menu
void handle_leave_god_mode(void*);

BOOL is_inventory_visible( void* user_data );
void handle_reset_view();

void disabled_duplicate(void*);
void handle_duplicate_in_place(void*);
void handle_repeat_duplicate(void*);

void handle_export(void*);
// void handle_deed_object_to_group(void*);
// BOOL enable_deed_object_to_group(void*);
void handle_object_owner_self(void*);
void handle_object_owner_permissive(void*);
void handle_object_lock(void*);
void handle_object_asset_ids(void*);
void force_take_copy(void*);
#ifdef _CORY_TESTING
void force_export_copy(void*);
void force_import_geometry(void*);
#endif

void handle_force_parcel_owner_to_me(void*);
void handle_force_parcel_to_content(void*);
void handle_claim_public_land(void*);

void handle_god_request_havok(void *);
void handle_god_request_avatar_geometry(void *);	// Hack for easy testing of new avatar geometry
void reload_personal_settings_overrides(void *);
void reload_vertex_shader(void *);
void slow_mo_animations(void *);
void handle_disconnect_viewer(void *);

void force_error_breakpoint(void *);
void force_error_llerror(void *);
void force_error_bad_memory_access(void *);
void force_error_infinite_loop(void *);
void force_error_software_exception(void *);
void force_error_driver_crash(void *);

void handle_stopall(void*);
//void handle_hinge(void*);
//void handle_ptop(void*);
//void handle_lptop(void*);
//void handle_wheel(void*);
//void handle_dehinge(void*);
BOOL enable_dehinge(void*);
void handle_force_delete(void*);
void print_object_info(void*);
void print_agent_nvpairs(void*);
void toggle_debug_menus(void*);
void export_info_callback(LLAssetInfo *info, void **user_data, S32 result);
void export_data_callback(LLVFS *vfs, const LLUUID& uuid, LLAssetType::EType type, void **user_data, S32 result);
void upload_done_callback(const LLUUID& uuid, void* user_data, S32 result, LLExtStat ext_status);
BOOL menu_check_build_tool( void* user_data );
void handle_reload_settings(void*);
void focus_here(void*);
void dump_select_mgr(void*);
void dump_volume_mgr(void*);
void dump_inventory(void*);
void edit_ui(void*);
void toggle_visibility(void*);
BOOL get_visibility(void*);

// Avatar Pie menu
void request_friendship(const LLUUID& agent_id);

// Tools menu
void handle_force_unlock(void*);
void handle_selected_texture_info(void*);
void handle_dump_image_list(void*);

void handle_crash(void*);
void handle_dump_followcam(void*);
void handle_viewer_enable_message_log(void*);
void handle_viewer_disable_message_log(void*);
void handle_send_postcard(void*);
void handle_gestures_old(void*);
void handle_focus(void *);
BOOL enable_buy_land(void*);
void handle_move(void*);
void handle_show_inventory(void*);
void handle_activate(void*);
BOOL enable_activate(void*);

// Help menu
void handle_buy_currency(void*);

void handle_test_male(void *);
void handle_test_female(void *);
void handle_toggle_pg(void*);
void handle_dump_attachments(void *);
void handle_show_overlay_title(void*);
void handle_dump_avatar_local_textures(void*);
void handle_debug_avatar_textures(void*);
void handle_grab_texture(void*);
BOOL enable_grab_texture(void*);
void handle_dump_region_object_cache(void*);

BOOL menu_ui_enabled(void *user_data);
void check_toggle_control( LLUICtrl *, void* user_data );
BOOL menu_check_control( void* user_data);
void menu_toggle_variable( void* user_data );
BOOL menu_check_variable( void* user_data);
BOOL enable_land_selected( void* );
BOOL enable_more_than_one_selected(void* );
BOOL enable_selection_you_own_all(void*);
BOOL enable_selection_you_own_one(void*);
BOOL enable_save_into_inventory(void*);
BOOL enable_save_into_task_inventory(void*);
BOOL enable_not_thirdperson(void*);
// BOOL enable_export_selected(void *);
BOOL enable_have_card(void*);
BOOL enable_detach(void*);
BOOL enable_region_owner(void*);
void menu_toggle_attached_lights(void* user_data);
void menu_toggle_attached_particles(void* user_data);
static void handle_go_to_callback(S32 option, void *userdata);

class LLMenuParcelObserver : public LLParcelObserver
{
public:
	LLMenuParcelObserver();
	~LLMenuParcelObserver();
	virtual void changed();
};

static LLMenuParcelObserver* gMenuParcelObserver = NULL;

LLMenuParcelObserver::LLMenuParcelObserver()
{
	LLViewerParcelMgr::getInstance()->addObserver(this);
}

LLMenuParcelObserver::~LLMenuParcelObserver()
{
	LLViewerParcelMgr::getInstance()->removeObserver(this);
}

void LLMenuParcelObserver::changed()
{
	gMenuHolder->childSetEnabled("Land Buy Pass", LLPanelLandGeneral::enableBuyPass(NULL));
	
	BOOL buyable = enable_buy_land(NULL);
	gMenuHolder->childSetEnabled("Land Buy", buyable);
	gMenuHolder->childSetEnabled("Buy Land...", buyable);
}


//-----------------------------------------------------------------------------
// Menu Construction
//-----------------------------------------------------------------------------

// code required to calculate anything about the menus
void pre_init_menus()
{
	// static information
	LLColor4 color;
	color = gColors.getColor( "MenuDefaultBgColor" );
	LLMenuGL::setDefaultBackgroundColor( color );
	color = gColors.getColor( "MenuItemEnabledColor" );
	LLMenuItemGL::setEnabledColor( color );
	color = gColors.getColor( "MenuItemDisabledColor" );
	LLMenuItemGL::setDisabledColor( color );
	color = gColors.getColor( "MenuItemHighlightBgColor" );
	LLMenuItemGL::setHighlightBGColor( color );
	color = gColors.getColor( "MenuItemHighlightFgColor" );
	LLMenuItemGL::setHighlightFGColor( color );
}

void initialize_menus();

//-----------------------------------------------------------------------------
// Initialize main menus
//
// HOW TO NAME MENUS:
//
// First Letter Of Each Word Is Capitalized, Even At Or And
//
// Items that lead to dialog boxes end in "..."
//
// Break up groups of more than 6 items with separators
//-----------------------------------------------------------------------------

void set_underclothes_menu_options()
{
	if (gMenuHolder && gAgent.isTeen())
	{
		gMenuHolder->getChild<LLView>("Self Underpants", TRUE)->setVisible(FALSE);
		gMenuHolder->getChild<LLView>("Self Undershirt", TRUE)->setVisible(FALSE);
	}
	if (gMenuBarView && gAgent.isTeen())
	{
		gMenuBarView->getChild<LLView>("Menu Underpants", TRUE)->setVisible(FALSE);
		gMenuBarView->getChild<LLView>("Menu Undershirt", TRUE)->setVisible(FALSE);
	}
}

void init_menus()
{
	if (gMenuHolder)
	{
		cleanup_menus();
	}

	S32 top = gViewerWindow->getRootView()->getRect().getHeight();
	S32 width = gViewerWindow->getRootView()->getRect().getWidth();

	//
	// Main menu bar
	//
	gMenuHolder = new LLViewerMenuHolderGL();
	gMenuHolder->setRect(LLRect(0, top, width, 0));
	gMenuHolder->setFollowsAll();

	LLMenuGL::sMenuContainer = gMenuHolder;

	// Initialize actions
	initialize_menus();

	///
	/// Popup menu
	///
	/// The popup menu is now populated by the show_context_menu()
	/// method.
	
	gPopupMenuView = new LLMenuGL( "Popup" );
	gPopupMenuView->setVisible( FALSE );
	gMenuHolder->addChild( gPopupMenuView );

	///
	/// Pie menus
	///
	gPieSelf = LLUICtrlFactory::getInstance()->buildPieMenu("menu_pie_self.xml", gMenuHolder);

	// TomY TODO: what shall we do about these?
	gDetachScreenPieMenu = gMenuHolder->getChild<LLPieMenu>("Object Detach HUD", true);
	gDetachPieMenu = gMenuHolder->getChild<LLPieMenu>("Object Detach", true);

	gPieAvatar = LLUICtrlFactory::getInstance()->buildPieMenu("menu_pie_avatar.xml", gMenuHolder);

	gPieObject = LLUICtrlFactory::getInstance()->buildPieMenu("menu_pie_object.xml", gMenuHolder);

	gAttachScreenPieMenu = gMenuHolder->getChild<LLPieMenu>("Object Attach HUD");
	gAttachPieMenu = gMenuHolder->getChild<LLPieMenu>("Object Attach");

	gPieAttachment = LLUICtrlFactory::getInstance()->buildPieMenu("menu_pie_attachment.xml", gMenuHolder);

	gPieHUD = LLUICtrlFactory::getInstance()->buildPieMenu("menu_pie_hud.xml", gMenuHolder);

	gPieLand = LLUICtrlFactory::getInstance()->buildPieMenu("menu_pie_land.xml", gMenuHolder);

	///
	/// set up the colors
	///
	LLColor4 color;

	LLColor4 pie_color = gColors.getColor("PieMenuBgColor");
	gPieSelf->setBackgroundColor( pie_color );
	gPieAvatar->setBackgroundColor( pie_color );
	gPieObject->setBackgroundColor( pie_color );
	gPieAttachment->setBackgroundColor( pie_color );
	gPieHUD->setBackgroundColor( pie_color );
	gPieLand->setBackgroundColor( pie_color );

	color = gColors.getColor( "MenuPopupBgColor" );
	gPopupMenuView->setBackgroundColor( color );

	// If we are not in production, use a different color to make it apparent.
	if (LLViewerLogin::getInstance()->isInProductionGrid())
	{
		color = gColors.getColor( "MenuBarBgColor" );
	}
	else
	{
		color = gColors.getColor( "MenuNonProductionBgColor" );
	}
	gMenuBarView = (LLMenuBarGL*)LLUICtrlFactory::getInstance()->buildMenu("menu_viewer.xml", gMenuHolder);
	gMenuBarView->setRect(LLRect(0, top, 0, top - MENU_BAR_HEIGHT));
	gMenuBarView->setBackgroundColor( color );

    // gMenuBarView->setItemVisible("Tools", FALSE);
	gMenuBarView->arrange();
	
	gMenuHolder->addChild(gMenuBarView);
	
	// menu holder appears on top of menu bar so you can see the menu title
	// flash when an item is triggered (the flash occurs in the holder)
	gViewerWindow->getRootView()->addChild(gMenuHolder);
   
    gViewerWindow->setMenuBackgroundColor(false, 
        LLViewerLogin::getInstance()->isInProductionGrid());

	// *TODO:Get the cost info from the server
	const std::string upload_cost("10");
	gMenuHolder->childSetLabelArg("Upload Image", "[COST]", upload_cost);
	gMenuHolder->childSetLabelArg("Upload Sound", "[COST]", upload_cost);
	gMenuHolder->childSetLabelArg("Upload Animation", "[COST]", upload_cost);
	gMenuHolder->childSetLabelArg("Bulk Upload", "[COST]", upload_cost);

	gAFKMenu = gMenuBarView->getChild<LLMenuItemCallGL>("Set Away", TRUE);
	gBusyMenu = gMenuBarView->getChild<LLMenuItemCallGL>("Set Busy", TRUE);
	gAttachSubMenu = gMenuBarView->getChildMenuByName("Attach Object", TRUE);
	gDetachSubMenu = gMenuBarView->getChildMenuByName("Detach Object", TRUE);

	// TomY TODO convert these two
	LLMenuGL*menu;

	// Advanced (Client) menu is XUI now! \o/
	/*
	menu = new LLMenuGL(CLIENT_MENU_NAME);
	init_client_menu(menu);
	gMenuBarView->appendMenu( menu );
	menu->updateParent(LLMenuGL::sMenuContainer);
	*/

	menu = new LLMenuGL(SERVER_MENU_NAME);
	init_server_menu(menu);
	gMenuBarView->appendMenu( menu );
	menu->updateParent(LLMenuGL::sMenuContainer);

	gMenuBarView->createJumpKeys();

	// Let land based option enable when parcel changes
	gMenuParcelObserver = new LLMenuParcelObserver();

	//
	// Debug menu visiblity
	//
	show_debug_menus();

	gLoginMenuBarView = (LLMenuBarGL*)LLUICtrlFactory::getInstance()->buildMenu("menu_login.xml", gMenuHolder);
	LLRect menuBarRect = gLoginMenuBarView->getRect();
	gLoginMenuBarView->setRect(LLRect(menuBarRect.mLeft, menuBarRect.mTop, gViewerWindow->getRootView()->getRect().getWidth() - menuBarRect.mLeft,  menuBarRect.mBottom));

	gLoginMenuBarView->setBackgroundColor( color );

	gMenuHolder->addChild(gLoginMenuBarView);
	
}



void init_client_menu(LLMenuGL* menu)
{
	LLMenuGL* sub_menu = NULL;

	//menu->append(new LLMenuItemCallGL("Permissions Control", &show_permissions_control));
	// this is now in the view menu so we don't need it here!
	
	{
		// *TODO: Translate
		LLMenuGL* sub = new LLMenuGL("Consoles");
		menu->appendMenu(sub);
		sub->append(new LLMenuItemCheckGL("Frame Console", 
										&toggle_visibility,
										NULL,
										&get_visibility,
										(void*)gDebugView->mFrameStatView,
										'2', MASK_CONTROL|MASK_SHIFT ) );
		sub->append(new LLMenuItemCheckGL("Texture Console", 
										&toggle_visibility,
										NULL,
										&get_visibility,
										(void*)gTextureView,
									   	'3', MASK_CONTROL|MASK_SHIFT ) );
		LLView* debugview = gDebugView->mDebugConsolep;
		sub->append(new LLMenuItemCheckGL("Debug Console", 
										&toggle_visibility,
										NULL,
										&get_visibility,
										debugview,
									   	'4', MASK_CONTROL|MASK_SHIFT ) );

		sub->append(new LLMenuItemCheckGL("Fast Timers", 
										&toggle_visibility,
										NULL,
										&get_visibility,
										(void*)gDebugView->mFastTimerView,
										  '9', MASK_CONTROL|MASK_SHIFT ) );
#if MEM_TRACK_MEM
		sub->append(new LLMenuItemCheckGL("Memory", 
										&toggle_visibility,
										NULL,
										&get_visibility,
										(void*)gDebugView->mMemoryView,
										  '0', MASK_CONTROL|MASK_SHIFT ) );
#endif
		sub->appendSeparator();
		sub->append(new LLMenuItemCallGL("Region Info to Debug Console", 
			&handle_region_dump_settings, NULL));
		sub->append(new LLMenuItemCallGL("Group Info to Debug Console",
			&handle_dump_group_info, NULL, NULL));
		sub->append(new LLMenuItemCallGL("Capabilities Info to Debug Console",
			&handle_dump_capabilities_info, NULL, NULL));
		sub->createJumpKeys();
	}
	
	// neither of these works particularly well at the moment
	/*menu->append(new LLMenuItemCallGL(  "Reload UI XML",	&reload_ui,	
	  				NULL, NULL) );*/
	/*menu->append(new LLMenuItemCallGL("Reload settings/colors", 
					&handle_reload_settings, NULL, NULL));*/
	menu->append(new LLMenuItemCallGL("Reload personal setting overrides", 
		&reload_personal_settings_overrides, NULL, NULL, KEY_F2, MASK_CONTROL|MASK_SHIFT));

	sub_menu = new LLMenuGL("HUD Info");
	{
		sub_menu->append(new LLMenuItemCheckGL("Velocity", 
												&toggle_visibility,
												NULL,
												&get_visibility,
												(void*)gVelocityBar));

		sub_menu->append(new LLMenuItemToggleGL("Camera",	&gDisplayCameraPos ) );
		sub_menu->append(new LLMenuItemToggleGL("Wind", 	&gDisplayWindInfo) );
		sub_menu->append(new LLMenuItemToggleGL("FOV",  	&gDisplayFOV ) );
		sub_menu->createJumpKeys();
	}
	menu->appendMenu(sub_menu);

	menu->appendSeparator();
	
	menu->append(new LLMenuItemCheckGL( "Quiet Snapshots to Disk",
										&menu_toggle_control,
										NULL,
										&menu_check_control,
										(void*)"QuietSnapshotsToDisk"));

	menu->append(new LLMenuItemCheckGL("Show Mouselook Crosshairs",
									   &menu_toggle_control,
									   NULL,
									   &menu_check_control,
									   (void*)"ShowCrosshairs"));

	menu->append(new LLMenuItemCheckGL("Debug Permissions",
									   &menu_toggle_control,
									   NULL,
									   &menu_check_control,
									   (void*)"DebugPermissions"));
	


#ifdef TOGGLE_HACKED_GODLIKE_VIEWER
	if (!LLViewerLogin::getInstance()->isInProductionGrid())
	{
		menu->append(new LLMenuItemCheckGL("Hacked Godmode",
										   &handle_toggle_hacked_godmode,
										   NULL,
										   &check_toggle_hacked_godmode,
										   (void*)"HackedGodmode"));
	}
#endif

	menu->append(new LLMenuItemCallGL("Clear Group Cache", 
									  LLGroupMgr::debugClearAllGroups));
	menu->appendSeparator();

	sub_menu = new LLMenuGL("Rendering");
	init_debug_rendering_menu(sub_menu);
	menu->appendMenu(sub_menu);

	sub_menu = new LLMenuGL("World");
	init_debug_world_menu(sub_menu);
	menu->appendMenu(sub_menu);

// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e) | Modified: RLVa-0.2.1b
	#ifdef RLV_ADVANCED_MENU
		if (rlv_handler_t::isEnabled())
		{
			sub_menu = new LLMenuGL("RLVa");
			init_debug_rlva_menu(sub_menu);
			menu->appendMenu(sub_menu);
			sub_menu->setVisible(rlv_handler_t::isEnabled());
			sub_menu->setEnabled(rlv_handler_t::isEnabled());
		}
	#endif // RLV_ADVANCED_MENU
// [/RLVa:KB]

	sub_menu = new LLMenuGL("UI");
	init_debug_ui_menu(sub_menu);
	menu->appendMenu(sub_menu);

	sub_menu = new LLMenuGL("XUI");
	init_debug_xui_menu(sub_menu);
	menu->appendMenu(sub_menu);

	sub_menu = new LLMenuGL("Character");
	init_debug_avatar_menu(sub_menu);
	menu->appendMenu(sub_menu);

{
		LLMenuGL* sub = NULL;
		sub = new LLMenuGL("Network");

		sub->append(new LLMenuItemCallGL("Enable Message Log",  
			&handle_viewer_enable_message_log,  NULL));
		sub->append(new LLMenuItemCallGL("Disable Message Log", 
			&handle_viewer_disable_message_log, NULL));

		sub->appendSeparator();

		sub->append(new LLMenuItemCheckGL("Velocity Interpolate Objects", 
										  &velocity_interpolate,
										  NULL, 
										  &menu_check_control,
										  (void*)"VelocityInterpolate"));
		sub->append(new LLMenuItemCheckGL("Ping Interpolate Object Positions", 
										  &menu_toggle_control,
										  NULL, 
										  &menu_check_control,
										  (void*)"PingInterpolate"));

		sub->appendSeparator();

		sub->append(new LLMenuItemCallGL("Drop a Packet", 
			&drop_packet, NULL, NULL, 
			'L', MASK_ALT | MASK_CONTROL));

		menu->appendMenu( sub );
		sub->createJumpKeys();
	}
	{
		LLMenuGL* sub = NULL;
		sub = new LLMenuGL("Recorder");

		sub->append(new LLMenuItemCheckGL("Full Session Logging", &menu_toggle_control, NULL, &menu_check_control, (void*)"StatsSessionTrackFrameStats"));

		sub->append(new LLMenuItemCallGL("Start Logging",	&LLFrameStats::startLogging, NULL));
		sub->append(new LLMenuItemCallGL("Stop Logging",	&LLFrameStats::stopLogging, NULL));
		sub->append(new LLMenuItemCallGL("Log 10 Seconds", &LLFrameStats::timedLogging10, NULL));
		sub->append(new LLMenuItemCallGL("Log 30 Seconds", &LLFrameStats::timedLogging30, NULL));
		sub->append(new LLMenuItemCallGL("Log 60 Seconds", &LLFrameStats::timedLogging60, NULL));
		sub->appendSeparator();
		sub->append(new LLMenuItemCallGL("Start Playback", &LLAgentPilot::startPlayback, NULL));
		sub->append(new LLMenuItemCallGL("Stop Playback",	&LLAgentPilot::stopPlayback, NULL));
		sub->append(new LLMenuItemToggleGL("Loop Playback", &LLAgentPilot::sLoop) );
		sub->append(new LLMenuItemCallGL("Start Record",	&LLAgentPilot::startRecord, NULL));
		sub->append(new LLMenuItemCallGL("Stop Record",	&LLAgentPilot::saveRecord, NULL));

		menu->appendMenu( sub );
		sub->createJumpKeys();
	}

	menu->appendSeparator();

	menu->append(new LLMenuItemToggleGL("Show Updates", 
		&gShowObjectUpdates));
	
	menu->appendSeparator(); 
	
	menu->append(new LLMenuItemCallGL("Compress Images...", 
		&handle_compress_image, NULL, NULL));

	menu->append(new LLMenuItemCheckGL("Limit Select Distance", 
									   &menu_toggle_control,
									   NULL, 
									   &menu_check_control,
									   (void*)"LimitSelectDistance"));

	menu->append(new LLMenuItemCheckGL("Mouse Smoothing",
										&menu_toggle_control,
										NULL,
										&menu_check_control,
										(void*) "MouseSmooth"));
	menu->appendSeparator();

	menu->append(new LLMenuItemCheckGL( "Console Window", 
										&menu_toggle_control,
										NULL, 
										&menu_check_control,
										(void*)"ShowConsoleWindow"));

// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e) | Modified: RLVa-1.0.0e
	#ifdef RLV_ADVANCED_TOGGLE_RLVA
		if (gSavedSettings.controlExists(RLV_SETTING_MAIN))
			menu->append(new LLMenuItemCheckGL("Restrained Life API", &rlvToggleEnabled, NULL, &rlvGetEnabled, NULL));
	#endif // RLV_ADVANCED_TOGGLE_RLVA
// [/RLVa:KB]

	if(gSavedSettings.getBOOL("QAMode"))
	{
		LLMenuGL* sub = NULL;
		sub = new LLMenuGL("Debugging");
#if LL_WINDOWS
        sub->append(new LLMenuItemCallGL("Force Breakpoint", &force_error_breakpoint, NULL, NULL, 'B', MASK_CONTROL | MASK_ALT));
#endif
		sub->append(new LLMenuItemCallGL("Force LLError And Crash", &force_error_llerror));
        sub->append(new LLMenuItemCallGL("Force Bad Memory Access", &force_error_bad_memory_access));
		sub->append(new LLMenuItemCallGL("Force Infinite Loop", &force_error_infinite_loop));
		sub->append(new LLMenuItemCallGL("Force Driver Crash", &force_error_driver_crash));
		sub->append(new LLMenuItemCallGL("Force Disconnect Viewer", &handle_disconnect_viewer));
		// *NOTE:Mani this isn't handled yet... sub->append(new LLMenuItemCallGL("Force Software Exception", &force_error_unhandled_exception)); 
		sub->createJumpKeys();
		menu->appendMenu(sub);
	}

	menu->append(new LLMenuItemCheckGL( "Output Debug Minidump", 
										&menu_toggle_control,
										NULL, 
										&menu_check_control,
										(void*)"SaveMinidump"));

	// TomY Temporary menu item so we can test this floater
	menu->append(new LLMenuItemCheckGL("Clothing...", 
												&handle_clothing,
												NULL,
												NULL,
												NULL));

	menu->append(new LLMenuItemCallGL("Debug Settings...", LLFloaterSettingsDebug::show, NULL, NULL));
	menu->append(new LLMenuItemCheckGL("View Admin Options", &handle_admin_override_toggle, NULL, &check_admin_override, NULL, 'V', MASK_CONTROL | MASK_ALT));

	menu->append(new LLMenuItemCallGL("Request Admin Status", 
		&handle_god_mode, NULL, NULL, 'G', MASK_ALT | MASK_CONTROL));

	menu->append(new LLMenuItemCallGL("Leave Admin Status", 
		&handle_leave_god_mode, NULL, NULL, 'G', MASK_ALT | MASK_SHIFT | MASK_CONTROL));

	menu->createJumpKeys();
}

void init_debug_world_menu(LLMenuGL* menu)
{
/* REMOVE mouse move sun from menu options
	menu->append(new LLMenuItemCheckGL("Mouse Moves Sun", 
									   &menu_toggle_control,
									   NULL, 
									   &menu_check_control,
									   (void*)"MouseSun", 
									   'M', MASK_CONTROL|MASK_ALT));
*/
	menu->append(new LLMenuItemCheckGL("Sim Sun Override", 
									   &menu_toggle_control,
									   NULL, 
									   &menu_check_control,
									   (void*)"SkyOverrideSimSunPosition"));
	menu->append(new LLMenuItemCallGL("Dump Scripted Camera",
		&handle_dump_followcam, NULL, NULL));
	menu->append(new LLMenuItemCheckGL("Fixed Weather", 
									   &menu_toggle_control,
									   NULL, 
									   &menu_check_control,
									   (void*)"FixedWeather"));
	menu->append(new LLMenuItemCallGL("Dump Region Object Cache",
		&handle_dump_region_object_cache, NULL, NULL));
	menu->createJumpKeys();
}


void handle_export_menus_to_xml(void*)
{
	LLFilePicker& picker = LLFilePicker::instance();
	if(!picker.getSaveFile(LLFilePicker::FFSAVE_XML))
	{
		llwarns << "No file" << llendl;
		return;
	}
	std::string filename = picker.getFirstFile();

	llofstream out(filename);
	LLXMLNodePtr node = gMenuBarView->getXML();
	node->writeToOstream(out);
	out.close();
}

extern BOOL gDebugClicks;
extern BOOL gDebugWindowProc;
extern BOOL gDebugTextEditorTips;
extern BOOL gDebugSelectMgr;

void init_debug_ui_menu(LLMenuGL* menu)
{
	menu->append(new LLMenuItemCallGL("SLURL Test", &handle_slurl_test));
	menu->append(new LLMenuItemCallGL("Editable UI", &edit_ui));
	menu->append(new LLMenuItemCallGL( "Dump SelectMgr", &dump_select_mgr));
	menu->append(new LLMenuItemCallGL( "Dump Inventory", &dump_inventory));
	menu->append(new LLMenuItemCallGL( "Dump Focus Holder", &handle_dump_focus, NULL, NULL, 'F', MASK_ALT | MASK_CONTROL));
	menu->append(new LLMenuItemCallGL( "Print Selected Object Info",	&print_object_info, NULL, NULL, 'P', MASK_CONTROL|MASK_SHIFT ));
	menu->append(new LLMenuItemCallGL( "Print Agent Info",			&print_agent_nvpairs, NULL, NULL, 'P', MASK_SHIFT ));
	menu->append(new LLMenuItemCallGL( "Memory Stats",  &output_statistics, NULL, NULL, 'M', MASK_SHIFT | MASK_ALT | MASK_CONTROL));
	menu->append(new LLMenuItemCheckGL("Double-Click Auto-Pilot", 
		menu_toggle_control, NULL, menu_check_control, 
		(void*)"DoubleClickAutoPilot"));
	menu->appendSeparator();
//	menu->append(new LLMenuItemCallGL( "Print Packets Lost",			&print_packets_lost, NULL, NULL, 'L', MASK_SHIFT ));
	menu->append(new LLMenuItemToggleGL("Debug SelectMgr", &gDebugSelectMgr));
	menu->append(new LLMenuItemToggleGL("Debug Clicks", &gDebugClicks));
	menu->append(new LLMenuItemToggleGL("Debug Views", &LLView::sDebugRects));
	menu->append(new LLMenuItemCheckGL("Show Name Tooltips", toggle_show_xui_names, NULL, check_show_xui_names, NULL));
	menu->append(new LLMenuItemToggleGL("Debug Mouse Events", &LLView::sDebugMouseHandling));
	menu->append(new LLMenuItemToggleGL("Debug Keys", &LLView::sDebugKeys));
	menu->append(new LLMenuItemToggleGL("Debug WindowProc", &gDebugWindowProc));
	menu->append(new LLMenuItemToggleGL("Debug Text Editor Tips", &gDebugTextEditorTips));
	menu->appendSeparator();
	menu->append(new LLMenuItemCheckGL("Show Time", menu_toggle_control, NULL, menu_check_control, (void*)"DebugShowTime"));
	menu->append(new LLMenuItemCheckGL("Show Render Info", menu_toggle_control, NULL, menu_check_control, (void*)"DebugShowRenderInfo"));
	menu->append(new LLMenuItemCheckGL("Show Color Under Cursor", menu_toggle_control, NULL, menu_check_control, (void*)"DebugShowColor"));
	
	menu->createJumpKeys();
}

void init_debug_xui_menu(LLMenuGL* menu)
{
	menu->append(new LLMenuItemCallGL("Floater Test...", LLFloaterTest::show));
	menu->append(new LLMenuItemCallGL("Export Menus to XML...", handle_export_menus_to_xml));
	menu->append(new LLMenuItemCallGL("Edit UI...", LLFloaterEditUI::show));	
	menu->append(new LLMenuItemCallGL("Load from XML...", handle_load_from_xml));
	menu->append(new LLMenuItemCallGL("Save to XML...", handle_save_to_xml));
	menu->append(new LLMenuItemCheckGL("Show XUI Names", toggle_show_xui_names, NULL, check_show_xui_names, NULL));

	//menu->append(new LLMenuItemCallGL("Buy Currency...", handle_buy_currency));
	menu->createJumpKeys();
}

void init_debug_rendering_menu(LLMenuGL* menu)
{
	LLMenuGL* sub_menu = NULL;

	///////////////////////////
	//
	// Debug menu for types/pools
	//
	sub_menu = new LLMenuGL("Types");
	menu->appendMenu(sub_menu);

	sub_menu->append(new LLMenuItemCheckGL("Simple",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_SIMPLE,	'1', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Alpha",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_ALPHA, '2', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Tree",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_TREE, '3', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Character",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_AVATAR, '4', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("SurfacePatch",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_TERRAIN, '5', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Sky",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_SKY, '6', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Water",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_WATER, '7', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Ground",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_GROUND, '8', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Volume",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_VOLUME, '9', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Grass",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_GRASS, '0', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Clouds",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_CLOUDS, '-', MASK_CONTROL|MASK_ALT| MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Particles",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_PARTICLES, '=', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->append(new LLMenuItemCheckGL("Bump",
											&LLPipeline::toggleRenderTypeControl, NULL,
											&LLPipeline::hasRenderTypeControl,
											(void*)LLPipeline::RENDER_TYPE_BUMP, '\\', MASK_CONTROL|MASK_ALT|MASK_SHIFT));
	sub_menu->createJumpKeys();
	sub_menu = new LLMenuGL("Features");
	menu->appendMenu(sub_menu);
	sub_menu->append(new LLMenuItemCheckGL("UI",
											&LLPipeline::toggleRenderDebugFeature, NULL,
											&LLPipeline::toggleRenderDebugFeatureControl,
											(void*)LLPipeline::RENDER_DEBUG_FEATURE_UI, KEY_F1, MASK_ALT|MASK_CONTROL));
	sub_menu->append(new LLMenuItemCheckGL("Selected",
											&LLPipeline::toggleRenderDebugFeature, NULL,
											&LLPipeline::toggleRenderDebugFeatureControl,
											(void*)LLPipeline::RENDER_DEBUG_FEATURE_SELECTED, KEY_F2, MASK_ALT|MASK_CONTROL));
	sub_menu->append(new LLMenuItemCheckGL("Highlighted",
											&LLPipeline::toggleRenderDebugFeature, NULL,
											&LLPipeline::toggleRenderDebugFeatureControl,
											(void*)LLPipeline::RENDER_DEBUG_FEATURE_HIGHLIGHTED, KEY_F3, MASK_ALT|MASK_CONTROL));
	sub_menu->append(new LLMenuItemCheckGL("Dynamic Textures",
											&LLPipeline::toggleRenderDebugFeature, NULL,
											&LLPipeline::toggleRenderDebugFeatureControl,
											(void*)LLPipeline::RENDER_DEBUG_FEATURE_DYNAMIC_TEXTURES, KEY_F4, MASK_ALT|MASK_CONTROL));
	sub_menu->append(new LLMenuItemCheckGL( "Foot Shadows", 
											&LLPipeline::toggleRenderDebugFeature, NULL,
											&LLPipeline::toggleRenderDebugFeatureControl,
											(void*)LLPipeline::RENDER_DEBUG_FEATURE_FOOT_SHADOWS, KEY_F5, MASK_ALT|MASK_CONTROL));
	sub_menu->append(new LLMenuItemCheckGL("Fog",
											&LLPipeline::toggleRenderDebugFeature, NULL,
											&LLPipeline::toggleRenderDebugFeatureControl,
											(void*)LLPipeline::RENDER_DEBUG_FEATURE_FOG, KEY_F6, MASK_ALT|MASK_CONTROL));
	sub_menu->append(new LLMenuItemCheckGL("Test FRInfo",
											&LLPipeline::toggleRenderDebugFeature, NULL,
											&LLPipeline::toggleRenderDebugFeatureControl,
											(void*)LLPipeline::RENDER_DEBUG_FEATURE_FR_INFO, KEY_F8, MASK_ALT|MASK_CONTROL));
	sub_menu->append(new LLMenuItemCheckGL( "Flexible Objects", 
											&LLPipeline::toggleRenderDebugFeature, NULL,
											&LLPipeline::toggleRenderDebugFeatureControl,
											(void*)LLPipeline::RENDER_DEBUG_FEATURE_FLEXIBLE, KEY_F9, MASK_ALT|MASK_CONTROL));
	sub_menu->createJumpKeys();

	/////////////////////////////
	//
	// Debug menu for info displays
	//
	sub_menu = new LLMenuGL("Info Displays");
	menu->appendMenu(sub_menu);

	sub_menu->append(new LLMenuItemCheckGL("Verify",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_VERIFY));
	sub_menu->append(new LLMenuItemCheckGL("BBoxes",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_BBOXES));
	sub_menu->append(new LLMenuItemCheckGL("Points",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_POINTS));
	sub_menu->append(new LLMenuItemCheckGL("Octree",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_OCTREE));
	sub_menu->append(new LLMenuItemCheckGL("Occlusion",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_OCCLUSION));
	sub_menu->append(new LLMenuItemCheckGL("Render Batches", &LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_BATCH_SIZE));
	sub_menu->append(new LLMenuItemCheckGL("Animated Textures",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_TEXTURE_ANIM));
	sub_menu->append(new LLMenuItemCheckGL("Texture Priority",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_TEXTURE_PRIORITY));
	sub_menu->append(new LLMenuItemCheckGL("Avatar Rendering Cost",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_SHAME));
	sub_menu->append(new LLMenuItemCheckGL("Texture Area (sqrt(A))",&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_TEXTURE_AREA));
	sub_menu->append(new LLMenuItemCheckGL("Face Area (sqrt(A))",&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_FACE_AREA));
	sub_menu->append(new LLMenuItemCheckGL("Lights",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_LIGHTS));
	sub_menu->append(new LLMenuItemCheckGL("Particles",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_PARTICLES));
	sub_menu->append(new LLMenuItemCheckGL("Composition", &LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_COMPOSITION));
	sub_menu->append(new LLMenuItemCheckGL("Glow",&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_GLOW));
	sub_menu->append(new LLMenuItemCheckGL("Raycasting",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_RAYCAST));
	sub_menu->append(new LLMenuItemCheckGL("Sculpt",	&LLPipeline::toggleRenderDebug, NULL,
													&LLPipeline::toggleRenderDebugControl,
													(void*)LLPipeline::RENDER_DEBUG_SCULPTED));
		
	sub_menu->append(new LLMenuItemCallGL("Vectorize Perf Test", &run_vectorize_perf_test));

	sub_menu = new LLMenuGL("Render Tests");

	sub_menu->append(new LLMenuItemCheckGL("Camera Offset", 
										  &menu_toggle_control,
										  NULL, 
										  &menu_check_control,
										  (void*)"CameraOffset"));

	sub_menu->append(new LLMenuItemToggleGL("Randomize Framerate", &gRandomizeFramerate));

	sub_menu->append(new LLMenuItemToggleGL("Periodic Slow Frame", &gPeriodicSlowFrame));

	sub_menu->append(new LLMenuItemToggleGL("Frame Test", &LLPipeline::sRenderFrameTest));

	sub_menu->createJumpKeys();

	menu->appendMenu( sub_menu );

	menu->appendSeparator();
	menu->append(new LLMenuItemCheckGL("Axes", menu_toggle_control, NULL, menu_check_control, (void*)"ShowAxes"));
//	menu->append(new LLMenuItemCheckGL("Cull Small Objects", toggle_cull_small, NULL, menu_check_control, (void*)"RenderCullBySize"));

	menu->appendSeparator();
	menu->append(new LLMenuItemToggleGL("Hide Selected", &gHideSelectedObjects));
	menu->appendSeparator();
	menu->append(new LLMenuItemCheckGL("Tangent Basis", menu_toggle_control, NULL, menu_check_control, (void*)"ShowTangentBasis"));
	menu->append(new LLMenuItemCallGL("Selected Texture Info", handle_selected_texture_info, NULL, NULL, 'T', MASK_CONTROL|MASK_SHIFT|MASK_ALT));
	//menu->append(new LLMenuItemCallGL("Dump Image List", handle_dump_image_list, NULL, NULL, 'I', MASK_CONTROL|MASK_SHIFT));
	
	menu->append(new LLMenuItemToggleGL("Wireframe", &gUseWireframe, 
			'R', MASK_CONTROL|MASK_SHIFT));

	LLMenuItemCheckGL* item;
	item = new LLMenuItemCheckGL("Object-Object Occlusion", menu_toggle_control, NULL, menu_check_control, (void*)"UseOcclusion", 'O', MASK_CONTROL|MASK_SHIFT);
	item->setEnabled(gGLManager.mHasOcclusionQuery && LLFeatureManager::getInstance()->isFeatureAvailable("UseOcclusion"));
	menu->append(item);

	item = new LLMenuItemCheckGL("Debug GL", menu_toggle_control, NULL, menu_check_control, (void*)"RenderDebugGL");
	menu->append(item);
	
	item = new LLMenuItemCheckGL("Debug Pipeline", menu_toggle_control, NULL, menu_check_control, (void*)"RenderDebugPipeline");
	menu->append(item);
	
	item = new LLMenuItemCheckGL("Fast Alpha", menu_toggle_control, NULL, menu_check_control, (void*)"RenderFastAlpha");
	menu->append(item);
	
	item = new LLMenuItemCheckGL("Animate Textures", menu_toggle_control, NULL, menu_check_control, (void*)"AnimateTextures");
	menu->append(item);
	
	item = new LLMenuItemCheckGL("Disable Textures", menu_toggle_variable, NULL, menu_check_variable, (void*)&LLViewerImage::sDontLoadVolumeTextures);
	menu->append(item);
	
#ifndef LL_RELEASE_FOR_DOWNLOAD
	item = new LLMenuItemCheckGL("HTTP Get Textures", menu_toggle_control, NULL, menu_check_control, (void*)"ImagePipelineUseHTTP");
	menu->append(item);
#endif
	
	item = new LLMenuItemCheckGL("Run Multiple Threads", menu_toggle_control, NULL, menu_check_control, (void*)"RunMultipleThreads");
	menu->append(item);

	item = new LLMenuItemCheckGL("Cheesy Beacon", menu_toggle_control, NULL, menu_check_control, (void*)"CheesyBeacon");
	menu->append(item);

	item = new LLMenuItemCheckGL("Attached Lights", menu_toggle_attached_lights, NULL, menu_check_control, (void*)"RenderAttachedLights");
	menu->append(item);

	item = new LLMenuItemCheckGL("Attached Particles", menu_toggle_attached_particles, NULL, menu_check_control, (void*)"RenderAttachedParticles");
	menu->append(item);

#ifndef LL_RELEASE_FOR_DOWNLOAD
	menu->appendSeparator();
	menu->append(new LLMenuItemCallGL("Memory Leaking Simulation", LLFloaterMemLeak::show, NULL, NULL));
#else
	if(gSavedSettings.getBOOL("QAMode"))
	{
		menu->appendSeparator();
		menu->append(new LLMenuItemCallGL("Memory Leaking Simulation", LLFloaterMemLeak::show, NULL, NULL));
	}
#endif
	
	menu->createJumpKeys();
}

extern BOOL gDebugAvatarRotation;

void init_debug_avatar_menu(LLMenuGL* menu)
{
	LLMenuGL* sub_menu = new LLMenuGL("Grab Baked Texture");
	init_debug_baked_texture_menu(sub_menu);
	menu->appendMenu(sub_menu);

	sub_menu = new LLMenuGL("Character Tests");
	sub_menu->append(new LLMenuItemToggleGL("Go Away/AFK When Idle",
		&gAllowIdleAFK));

	sub_menu->append(new LLMenuItemCallGL("Appearance To XML", 
		&LLVOAvatar::dumpArchetypeXML));

	// HACK for easy testing of avatar geometry
	sub_menu->append(new LLMenuItemCallGL( "Toggle Character Geometry", 
		&handle_god_request_avatar_geometry, &enable_god_customer_service, NULL));

	sub_menu->append(new LLMenuItemCallGL("Test Male", 
		handle_test_male));

	sub_menu->append(new LLMenuItemCallGL("Test Female", 
		handle_test_female));

	sub_menu->append(new LLMenuItemCallGL("Toggle PG", handle_toggle_pg));

	sub_menu->append(new LLMenuItemToggleGL("Allow Select Avatar", &gAllowSelectAvatar));
	sub_menu->createJumpKeys();

	menu->appendMenu(sub_menu);

	menu->append(new LLMenuItemCheckGL("Enable Lip Sync (Beta)", menu_toggle_control, NULL, menu_check_control, (void*)"LipSyncEnabled"));
	menu->append(new LLMenuItemToggleGL("Tap-Tap-Hold To Run", &gAllowTapTapHoldRun));
	menu->append(new LLMenuItemCallGL("Force Params to Default", &LLAgent::clearVisualParams, NULL));
	menu->append(new LLMenuItemCallGL("Reload Vertex Shader", &reload_vertex_shader, NULL));
	menu->append(new LLMenuItemToggleGL("Animation Info", &LLVOAvatar::sShowAnimationDebug));
	menu->append(new LLMenuItemCallGL("Slow Motion Animations", &slow_mo_animations, NULL));
	menu->append(new LLMenuItemToggleGL("Show Look At", &LLHUDEffectLookAt::sDebugLookAt));
	menu->append(new LLMenuItemToggleGL("Show Point At", &LLHUDEffectPointAt::sDebugPointAt));
	menu->append(new LLMenuItemToggleGL("Debug Joint Updates", &LLVOAvatar::sJointDebug));
	menu->append(new LLMenuItemToggleGL("Disable LOD", &LLViewerJoint::sDisableLOD));
	menu->append(new LLMenuItemToggleGL("Debug Character Vis", &LLVOAvatar::sDebugInvisible));
	//menu->append(new LLMenuItemToggleGL("Show Attachment Points", &LLVOAvatar::sShowAttachmentPoints));
	//diabling collision plane due to DEV-14477 -brad
	//menu->append(new LLMenuItemToggleGL("Show Collision Plane", &LLVOAvatar::sShowFootPlane));
	menu->append(new LLMenuItemToggleGL("Show Collision Skeleton", &LLVOAvatar::sShowCollisionVolumes));
	menu->append(new LLMenuItemToggleGL( "Display Agent Target", &LLAgent::sDebugDisplayTarget));
	menu->append(new LLMenuItemToggleGL( "Debug Rotation", &gDebugAvatarRotation));
	menu->append(new LLMenuItemCallGL("Dump Attachments", handle_dump_attachments));
	menu->append(new LLMenuItemCallGL("Refresh Appearance", handle_rebake_textures, NULL, NULL, 'R', MASK_ALT | MASK_CONTROL ));
#ifndef LL_RELEASE_FOR_DOWNLOAD
	menu->append(new LLMenuItemCallGL("Debug Avatar Textures", handle_debug_avatar_textures, NULL, NULL, 'A', MASK_SHIFT|MASK_CONTROL|MASK_ALT));
	menu->append(new LLMenuItemCallGL("Dump Local Textures", handle_dump_avatar_local_textures, NULL, NULL, 'M', MASK_SHIFT|MASK_ALT ));	
#endif
	menu->createJumpKeys();
}

void init_debug_baked_texture_menu(LLMenuGL* menu)
{
	menu->append(new LLMenuItemCallGL("Iris", handle_grab_texture, enable_grab_texture, (void*) LLVOAvatar::TEX_EYES_BAKED));
	menu->append(new LLMenuItemCallGL("Head", handle_grab_texture, enable_grab_texture, (void*) LLVOAvatar::TEX_HEAD_BAKED));
	menu->append(new LLMenuItemCallGL("Upper Body", handle_grab_texture, enable_grab_texture, (void*) LLVOAvatar::TEX_UPPER_BAKED));
	menu->append(new LLMenuItemCallGL("Lower Body", handle_grab_texture, enable_grab_texture, (void*) LLVOAvatar::TEX_LOWER_BAKED));
	menu->append(new LLMenuItemCallGL("Skirt", handle_grab_texture, enable_grab_texture, (void*) LLVOAvatar::TEX_SKIRT_BAKED));
	menu->createJumpKeys();
}

// [RLVa:KB] - Version: 1.22.11 | Checked: 2009-07-10 (RLVa-1.0.0g) | Modified: RLVa-1.0.0g
void init_debug_rlva_menu(LLMenuGL* menu)
{
	// Debug options
	{
		LLMenuGL* pDbgMenu = new LLMenuGL("Debug");

		if (gSavedSettings.controlExists(RLV_SETTING_DEBUG))
			pDbgMenu->append(new LLMenuItemCheckGL("Show Debug Messages", menu_toggle_control, NULL, menu_check_control, (void*)RLV_SETTING_DEBUG));
		pDbgMenu->appendSeparator();
		if (gSavedSettings.controlExists(RLV_SETTING_ENABLELEGACYNAMING))
			pDbgMenu->append(new LLMenuItemCheckGL("Enable Legacy Naming", menu_toggle_control, NULL, menu_check_control, (void*)RLV_SETTING_ENABLELEGACYNAMING));

		menu->appendMenu(pDbgMenu);
		menu->appendSeparator();
	}

	#ifdef RLV_EXTENSION_ENABLE_WEAR
		if (gSavedSettings.controlExists(RLV_SETTING_ENABLEWEAR))
			menu->append(new LLMenuItemCheckGL("Enable Wear", menu_toggle_control, rlvEnableWearEnabler, menu_check_control, (void*)RLV_SETTING_ENABLEWEAR));
		menu->appendSeparator();
	#endif // RLV_EXTENSION_ENABLE_WEAR

	#ifdef RLV_EXTENSION_HIDELOCKED
		if ( (gSavedSettings.controlExists(RLV_SETTING_HIDELOCKEDLAYER)) && 
			 (gSavedSettings.controlExists(RLV_SETTING_HIDELOCKEDATTACH)) )
		{
			menu->append(new LLMenuItemCheckGL("Hide locked layers", menu_toggle_control, NULL, menu_check_control, (void*)RLV_SETTING_HIDELOCKEDLAYER));
			menu->append(new LLMenuItemCheckGL("Hide locked attachments", menu_toggle_control, NULL, menu_check_control, (void*)RLV_SETTING_HIDELOCKEDATTACH));
			//sub_menu->append(new LLMenuItemToggleGL("Hide locked inventory", &rlv_handler_t::fHideLockedInventory));
			menu->appendSeparator();
		}
	#endif // RLV_EXTENSION_HIDELOCKED

	if (gSavedSettings.controlExists(RLV_SETTING_FORBIDGIVETORLV))
		menu->append(new LLMenuItemCheckGL("Forbid Give to #RLV", menu_toggle_control, NULL, menu_check_control, (void*)RLV_SETTING_FORBIDGIVETORLV));
	if (gSavedSettings.controlExists(RLV_SETTING_ENABLELEGACYNAMING))
		menu->append(new LLMenuItemCheckGL("Show Name Tags", menu_toggle_control, NULL, menu_check_control, (void*)RLV_SETTING_SHOWNAMETAGS));
	menu->appendSeparator();

	#ifdef RLV_EXTENSION_FLOATER_RESTRICTIONS
		// TODO-RLVa: figure out a way to tell if floater_rlv_behaviour.xml exists
		menu->append(new LLMenuItemCallGL("Restrictions...", RlvFloaterBehaviour::show, NULL, NULL));
	#endif // RLV_EXTENSION_FLOATER_RESTRICTIONS
}
// [/RLVa:KB]

void init_server_menu(LLMenuGL* menu)
{
	{
		LLMenuGL* sub = new LLMenuGL("Object");
		menu->appendMenu(sub);

		sub->append(new LLMenuItemCallGL( "Take Copy",
										  &force_take_copy, &enable_god_customer_service, NULL,
										  'O', MASK_SHIFT | MASK_ALT | MASK_CONTROL));
#ifdef _CORY_TESTING
		sub->append(new LLMenuItemCallGL( "Export Copy",
										   &force_export_copy, NULL, NULL));
		sub->append(new LLMenuItemCallGL( "Import Geometry",
										   &force_import_geometry, NULL, NULL));
#endif
		//sub->append(new LLMenuItemCallGL( "Force Public", 
		//			&handle_object_owner_none, NULL, NULL));
		//sub->append(new LLMenuItemCallGL( "Force Ownership/Permissive", 
		//			&handle_object_owner_self_and_permissive, NULL, NULL, 'K', MASK_SHIFT | MASK_ALT | MASK_CONTROL));
		sub->append(new LLMenuItemCallGL( "Force Owner To Me", 
					&handle_object_owner_self, &enable_god_customer_service));
		sub->append(new LLMenuItemCallGL( "Force Owner Permissive", 
					&handle_object_owner_permissive, &enable_god_customer_service));
		//sub->append(new LLMenuItemCallGL( "Force Totally Permissive", 
		//			&handle_object_permissive));
		sub->append(new LLMenuItemCallGL( "Delete", 
					&handle_force_delete, &enable_god_customer_service, NULL, KEY_DELETE, MASK_SHIFT | MASK_ALT | MASK_CONTROL));
		sub->append(new LLMenuItemCallGL( "Lock", 
					&handle_object_lock, &enable_god_customer_service, NULL, 'L', MASK_SHIFT | MASK_ALT | MASK_CONTROL));
		sub->append(new LLMenuItemCallGL( "Get Asset IDs", 
					&handle_object_asset_ids, &enable_god_customer_service, NULL, 'I', MASK_SHIFT | MASK_ALT | MASK_CONTROL));
		sub->createJumpKeys();
	}
	{
		LLMenuGL* sub = new LLMenuGL("Parcel");
		menu->appendMenu(sub);

		sub->append(new LLMenuItemCallGL("Owner To Me",
										 &handle_force_parcel_owner_to_me,
										 &enable_god_customer_service, NULL));
		sub->append(new LLMenuItemCallGL("Set to Linden Content",
										 &handle_force_parcel_to_content,
										 &enable_god_customer_service, NULL,
										 'C', MASK_SHIFT | MASK_ALT | MASK_CONTROL));
		sub->appendSeparator();
		sub->append(new LLMenuItemCallGL("Claim Public Land",
										 &handle_claim_public_land, &enable_god_customer_service));

		sub->createJumpKeys();
	}
	{
		LLMenuGL* sub = new LLMenuGL("Region");
		menu->appendMenu(sub);
		sub->append(new LLMenuItemCallGL("Dump Temp Asset Data",
			&handle_region_dump_temp_asset_data,
			&enable_god_customer_service, NULL));
		sub->createJumpKeys();
	}	
	menu->append(new LLMenuItemCallGL( "God Tools...", 
		&LLFloaterGodTools::show, &enable_god_basic, NULL));

	menu->appendSeparator();

	menu->append(new LLMenuItemCallGL("Save Region State", 
		&LLPanelRegionTools::onSaveState, &enable_god_customer_service, NULL));

//	menu->append(new LLMenuItemCallGL("Force Join Group", handle_force_join_group));
//
//	menu->appendSeparator();
//
//	menu->append(new LLMenuItemCallGL( "OverlayTitle",
//		&handle_show_overlay_title, &enable_god_customer_service, NULL));
	menu->createJumpKeys();
}

static std::vector<LLPointer<view_listener_t> > sMenus;

//-----------------------------------------------------------------------------
// cleanup_menus()
//-----------------------------------------------------------------------------
void cleanup_menus()
{
	delete gMenuParcelObserver;
	gMenuParcelObserver = NULL;

	delete gPieSelf;
	gPieSelf = NULL;

	delete gPieAvatar;
	gPieAvatar = NULL;

	delete gPieObject;
	gPieObject = NULL;

	delete gPieAttachment;
	gPieAttachment = NULL;

	delete gPieHUD;
	gPieHUD = NULL;

	delete gPieLand;
	gPieLand = NULL;

	delete gMenuBarView;
	gMenuBarView = NULL;

	delete gPopupMenuView;
	gPopupMenuView = NULL;

	delete gMenuHolder;
	gMenuHolder = NULL;

	sMenus.clear();
}

//-----------------------------------------------------------------------------
// Object pie menu
//-----------------------------------------------------------------------------

class LLObjectReportAbuse : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerObject* objectp = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		if (objectp)
		{
			LLFloaterReporter::showFromObject(objectp->getID());
		}
		return true;
	}
};

// Enabled it you clicked an object
class LLObjectEnableReportAbuse : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLSelectMgr::getInstance()->getSelection()->getObjectCount() != 0;
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLObjectTouch : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerObject* object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		if (!object) return true;

		LLPickInfo pick = LLToolPie::getInstance()->getPick();

// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g) | Added: RLVa-0.2.0f
		// TODO-RLVa: this code is rather redundant since we'll never get an active selection to show a pie menu for
		// [msg->addVector3("Position", pick.mIntersection) <- see llDetectedTouchPos()]
		if ( (gRlvHandler.hasBehaviour(RLV_BHVR_FARTOUCH)) && ((!object->isAttachment()) || (!object->permYouOwner())) &&
			 (dist_vec_squared(gAgent.getPositionAgent(), pick.mIntersection) > 1.5f * 1.5f)	)
		{
			return true;	// Can't touch in-world objects (or other avie's attachments) farther than 1.5m away under @fartouch=n
		}
// [/RLVa:KB]

		LLMessageSystem	*msg = gMessageSystem;

		msg->newMessageFast(_PREHASH_ObjectGrab);
		msg->nextBlockFast( _PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		msg->nextBlockFast( _PREHASH_ObjectData);
		msg->addU32Fast(    _PREHASH_LocalID, object->mLocalID);
		msg->addVector3Fast(_PREHASH_GrabOffset, LLVector3::zero );
		msg->nextBlock("SurfaceInfo");
		msg->addVector3("UVCoord", LLVector3(pick.mUVCoords));
		msg->addVector3("STCoord", LLVector3(pick.mSTCoords));
		msg->addS32Fast(_PREHASH_FaceIndex, pick.mObjectFace);
		msg->addVector3("Position", pick.mIntersection);
		msg->addVector3("Normal", pick.mNormal);
		msg->addVector3("Binormal", pick.mBinormal);
		msg->sendMessage( object->getRegion()->getHost());

		// *NOTE: Hope the packets arrive safely and in order or else
		// there will be some problems.
		// *TODO: Just fix this bad assumption.
		msg->newMessageFast(_PREHASH_ObjectDeGrab);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		msg->nextBlockFast(_PREHASH_ObjectData);
		msg->addU32Fast(_PREHASH_LocalID, object->mLocalID);
		msg->nextBlock("SurfaceInfo");
		msg->addVector3("UVCoord", LLVector3(pick.mUVCoords));
		msg->addVector3("STCoord", LLVector3(pick.mSTCoords));
		msg->addS32Fast(_PREHASH_FaceIndex, pick.mObjectFace);
		msg->addVector3("Position", pick.mIntersection);
		msg->addVector3("Normal", pick.mNormal);
		msg->addVector3("Binormal", pick.mBinormal);
		msg->sendMessage(object->getRegion()->getHost());

		return true;
	}
};


// One object must have touch sensor
class LLObjectEnableTouch : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerObject* obj = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		bool new_value = obj && obj->flagHandleTouch();
// [RLVa:KB] - Version: 1.22.11 | Checked: 2009-07-10 (RLVa-1.0.0g) | Added: RLVa-0.2.0f
		// TODO-RLVa: this code is rather redundant since we'll never get an active selection to show a pie menu for
		if ( (new_value) && (gRlvHandler.hasBehaviour(RLV_BHVR_FARTOUCH)) && ((!obj->isAttachment()) || (!obj->permYouOwner())) &&
			 (dist_vec_squared(gAgent.getPositionAgent(), LLToolPie::getInstance()->getPick().mIntersection) > 1.5f * 1.5f)	)
		{
			new_value = false;	// Can't touch in-world objects (or other avie's attachments) farther than 1.5m away under @fartouch=n
		}
// [/RLVa:KB]
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);

		// Update label based on the node touch name if available.
		LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode();
		if (node && node->mValid && !node->mTouchName.empty())
		{
			gMenuHolder->childSetText("Object Touch", node->mTouchName);
		}
		else
		{
			gMenuHolder->childSetText("Object Touch", userdata["data"].asString());
		}

		return true;
	}
};

void label_touch(std::string& label, void*)
{
	LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode();
	if (node && node->mValid && !node->mTouchName.empty())
	{
		label.assign(node->mTouchName);
	}
	else
	{
		label.assign("Touch");
	}
}

class LLAttachmentEnableTouch : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerObject* obj = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();

		if (!obj) return false;
		if (!obj->isAttachment()) return false;		

		bool new_value = obj && obj->flagHandleTouch();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode();

		if (node && node->mValid && !node->mTouchName.empty())
		{
			gMenuHolder->childSetText("Attachment Touch", node->mTouchName);
		}
		else
		{
			gMenuHolder->childSetText("Attachment Touch", userdata["data"].asString());
		}
		return true;
	}
};

bool handle_object_open()
{
	LLViewerObject* obj = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
	if(!obj) return true;

	LLFloaterOpenObject::show();
	return true;
}

class LLObjectOpen : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-04 (RLVa-1.0.0b)
		// TODO-RLVa: shouldn't we be checking for fartouch here as well?
		if (gRlvHandler.hasBehaviour(RLV_BHVR_EDIT))
		{
			return true;
		}
// [/RLVa:KB]

		return handle_object_open();
	}
};

class LLObjectEnableOpen : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// Look for contents in root object, which is all the LLFloaterOpenObject
		// understands.
		LLViewerObject* obj = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		bool new_value = (obj != NULL);
		if (new_value)
		{
			LLViewerObject* root = obj->getRootEdit();
			if (!root) new_value = false;
			else new_value = root->allowOpen();
		}

// [RLVa:KB] - Checked: 2009-07-04 (RLVa-1.0.0b) | Modified: RLVa-1.0.0b
		// TODO-RLV: shouldn't we be checking for fartouch here as well? (and LLViewerObject::allowOpen() makes this redundant?)
		new_value &= !gRlvHandler.hasBehaviour(RLV_BHVR_EDIT);
// [/RLVa:KB]

		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLViewCheckBuildMode : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLToolMgr::getInstance()->inEdit();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

bool toggle_build_mode()
{
	if (LLToolMgr::getInstance()->inBuildMode())
	{
		if (gSavedSettings.getBOOL("EditCameraMovement"))
		{
			// just reset the view, will pull us out of edit mode
			handle_reset_view();
		}
		else
		{
			// manually disable edit mode, but do not affect the camera
			gAgent.resetView(false);
			gFloaterTools->close();
			gViewerWindow->showCursor();			
		}
		// avoid spurious avatar movements pulling out of edit mode
		LLViewerJoystick::getInstance()->setNeedsReset();
	}
	else
	{
		ECameraMode camMode = gAgent.getCameraMode();
		if (CAMERA_MODE_MOUSELOOK == camMode ||	CAMERA_MODE_CUSTOMIZE_AVATAR == camMode)
		{
			// pull the user out of mouselook or appearance mode when entering build mode
			handle_reset_view();
		}

		if (gSavedSettings.getBOOL("EditCameraMovement"))
		{
			// camera should be set
			if (LLViewerJoystick::getInstance()->getOverrideCamera())
			{
				handle_toggle_flycam();
			}
				
			if (gAgent.getFocusOnAvatar())
			{
				// zoom in if we're looking at the avatar
				gAgent.setFocusOnAvatar(FALSE, ANIMATE);
				gAgent.setFocusGlobal(gAgent.getPositionGlobal() + 2.0 * LLVector3d(gAgent.getAtAxis()));
				gAgent.cameraZoomIn(0.666f);
				gAgent.cameraOrbitOver( 30.f * DEG_TO_RAD );
			}
		}

// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		if ( (gRlvHandler.hasBehaviour(RLV_BHVR_EDIT)) && (LLSelectMgr::getInstance()) )
		{
			LLSelectMgr::getInstance()->deselectAll();
		}
// [/RLVa:KB]

		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
		LLToolMgr::getInstance()->getCurrentToolset()->selectTool( LLToolCompCreate::getInstance() );

		// Could be first use
		LLFirstUse::useBuild();

		gAgent.resetView(false);

		// avoid spurious avatar movements
		LLViewerJoystick::getInstance()->setNeedsReset();

	}
	return true;
}

class LLViewBuildMode : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		return toggle_build_mode();
	}
};


class LLViewJoystickFlycam : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_toggle_flycam();
		return true;
	}
};

class LLViewCheckJoystickFlycam : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_val = LLViewerJoystick::getInstance()->getOverrideCamera();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_val);
		return true;
	}
};

class LLViewCommunicate : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (LLFloaterChatterBox::getInstance()->getFloaterCount() == 0)
		{
			LLFloaterMyFriends::toggleInstance();
		}
		else
		{
			LLFloaterChatterBox::toggleInstance();
		}
		return true;
	}
};


void handle_toggle_flycam()
{
	LLViewerJoystick::getInstance()->toggleFlycam();
}

class LLObjectBuild : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (gAgent.getFocusOnAvatar() && !LLToolMgr::getInstance()->inEdit() && gSavedSettings.getBOOL("EditCameraMovement") )
		{
			// zoom in if we're looking at the avatar
			gAgent.setFocusOnAvatar(FALSE, ANIMATE);
			gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());
			gAgent.cameraZoomIn(0.666f);
			gAgent.cameraOrbitOver( 30.f * DEG_TO_RAD );
			gViewerWindow->moveCursorToCenter();
		}
		else if ( gSavedSettings.getBOOL("EditCameraMovement") )
		{
			gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());
			gViewerWindow->moveCursorToCenter();
		}

		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
		LLToolMgr::getInstance()->getCurrentToolset()->selectTool( LLToolCompCreate::getInstance() );

		// Could be first use
		LLFirstUse::useBuild();
		return true;
	}
};

class LLObjectEdit : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerParcelMgr::getInstance()->deselectLand();

// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g) | Modified: RLVa-0.2.0f
		if (rlv_handler_t::isEnabled())
		{
			if (gRlvHandler.hasBehaviour(RLV_BHVR_EDIT))
			{
				return true;	// Can't edit any object under @edit=n
			}
			else if ( (gRlvHandler.hasBehaviour(RLV_BHVR_FARTOUCH)) &&
			          (SELECT_TYPE_WORLD == LLSelectMgr::getInstance()->getSelection()->getSelectType()) &&
					  (dist_vec_squared(gAgent.getPositionAgent(), LLToolPie::getInstance()->getPick().mIntersection) > 1.5f * 1.5f) )
			{
				// TODO-RLVa: this code is rather redundant since we'll never get an active selection to show a pie menu for
				return true;	// Can't edit in-world objects farther than 1.5m away under @fartouch=n
			}
		}
// [/RLVa:KB]

		if (gAgent.getFocusOnAvatar() && !LLToolMgr::getInstance()->inEdit())
		{
			LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();

			if (selection->getSelectType() == SELECT_TYPE_HUD || !gSavedSettings.getBOOL("EditCameraMovement"))
			{
				// always freeze camera in space, even if camera doesn't move
				// so, for example, follow cam scripts can't affect you when in build mode
				gAgent.setFocusGlobal(gAgent.calcFocusPositionTargetGlobal(), LLUUID::null);
				gAgent.setFocusOnAvatar(FALSE, ANIMATE);
			}
			else
			{
				gAgent.setFocusOnAvatar(FALSE, ANIMATE);
				LLViewerObject* selected_objectp = selection->getFirstRootObject();
				if (selected_objectp)
				{
				// zoom in on object center instead of where we clicked, as we need to see the manipulator handles
					gAgent.setFocusGlobal(selected_objectp->getPositionGlobal(), selected_objectp->getID());
				gAgent.cameraZoomIn(0.666f);
				gAgent.cameraOrbitOver( 30.f * DEG_TO_RAD );
				gViewerWindow->moveCursorToCenter();
			}
		}
		}

		gFloaterTools->open();		/* Flawfinder: ignore */
	
		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
		gFloaterTools->setEditTool( LLToolCompTranslate::getInstance() );

		LLViewerJoystick::getInstance()->moveObjects(true);
		LLViewerJoystick::getInstance()->setNeedsReset(true);

		// Could be first use
		LLFirstUse::useBuild();
		return true;
	}
};

class LLObjectInspect : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLFloaterInspect::show();
		return true;
	}
};


//---------------------------------------------------------------------------
// Land pie menu
//---------------------------------------------------------------------------
class LLLandBuild : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerParcelMgr::getInstance()->deselectLand();

		if (gAgent.getFocusOnAvatar() && !LLToolMgr::getInstance()->inEdit() && gSavedSettings.getBOOL("EditCameraMovement") )
		{
			// zoom in if we're looking at the avatar
			gAgent.setFocusOnAvatar(FALSE, ANIMATE);
			gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());
			gAgent.cameraZoomIn(0.666f);
			gAgent.cameraOrbitOver( 30.f * DEG_TO_RAD );
			gViewerWindow->moveCursorToCenter();
		}
		else if ( gSavedSettings.getBOOL("EditCameraMovement")  )
		{
			// otherwise just move focus
			gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());
			gViewerWindow->moveCursorToCenter();
		}


		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
		LLToolMgr::getInstance()->getCurrentToolset()->selectTool( LLToolCompCreate::getInstance() );

		// Could be first use
		LLFirstUse::useBuild();
		return true;
	}
};

class LLLandBuyPass : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLPanelLandGeneral::onClickBuyPass((void *)FALSE);
		return true;
	}
};

class LLLandEnableBuyPass : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLPanelLandGeneral::enableBuyPass(NULL);
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

// BUG: Should really check if CLICK POINT is in a parcel where you can build.
BOOL enable_land_build(void*)
{
	if (gAgent.isGodlike()) return TRUE;
	if (gAgent.inPrelude()) return FALSE;

	BOOL can_build = FALSE;
	LLParcel* agent_parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
	if (agent_parcel)
	{
		can_build = agent_parcel->getAllowModify();
	}
	return can_build;
}

// BUG: Should really check if OBJECT is in a parcel where you can build.
BOOL enable_object_build(void*)
{
	if (gAgent.isGodlike()) return TRUE;
	if (gAgent.inPrelude()) return FALSE;

	BOOL can_build = FALSE;
	LLParcel* agent_parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
	if (agent_parcel)
	{
		can_build = agent_parcel->getAllowModify();
	}
	return can_build;
}

class LLEnableEdit : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// *HACK:  The new "prelude" Help Islands have a build sandbox area,
		// so users need the Edit and Create pie menu options when they are
		// there.  Eventually this needs to be replaced with code that only 
		// lets you edit objects if you have permission to do so (edit perms,
		// group edit, god).  See also lltoolbar.cpp.  JC
		bool enable = true;
		if (gAgent.inPrelude())
		{
			enable = LLViewerParcelMgr::getInstance()->agentCanBuild()
				|| LLSelectMgr::getInstance()->getSelection()->isAttachment();
		}
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		// TODO-RLV: include fartouch here?
		if ( (rlv_handler_t::isEnabled()) && (enable) )
		{
			// We have no way of knowing whether we're being called for "Create" or for "Edit", but we can
			// make an educated guess based on the currently active selection which puts us halfway there.
			BOOL fActiveSelection = LLSelectMgr::getInstance()->getSelection()->getObjectCount();

			if ( (gRlvHandler.hasBehaviour(RLV_BHVR_EDIT)) && (gRlvHandler.hasBehaviour(RLV_BHVR_REZ)) )
				enable = false;	// Edit and rez restricted, disable them both
			else if ( (gRlvHandler.hasBehaviour(RLV_BHVR_EDIT)) && (fActiveSelection) )
				enable = false; // Edit restricted and there's an active selection => disable Edit and Create
			else if ( (gRlvHandler.hasBehaviour(RLV_BHVR_REZ)) && (!fActiveSelection) )
				enable = false; // Rez restricted and there's no active selection => disable Create
		}
// [/RLVa:KB]
		gMenuHolder->findControl(userdata["control"].asString())->setValue(enable);
		return true;
	}
};

class LLSelfRemoveAllAttachments : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLAgent::userRemoveAllAttachments(NULL);
		return true;
	}
};

class LLSelfEnableRemoveAllAttachments : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = false;
		if (gAgent.getAvatarObject())
		{
			LLVOAvatar* avatarp = gAgent.getAvatarObject();
			for (LLVOAvatar::attachment_map_t::iterator iter = avatarp->mAttachmentPoints.begin(); 
				 iter != avatarp->mAttachmentPoints.end(); )
			{
				LLVOAvatar::attachment_map_t::iterator curiter = iter++;
				LLViewerJointAttachment* attachment = curiter->second;
//				if (attachment->getObject())
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c) | Modified: RLVa-0.2.0c
				if ( (attachment->getObject()) && ( (!rlv_handler_t::isEnabled()) || (gRlvHandler.isDetachable(curiter->first)) ) )
// [/RLVa:KB]
				{
					new_value = true;
					break;
				}
			}
		}
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

BOOL enable_has_attachments(void*)
{

	return FALSE;
}

//---------------------------------------------------------------------------
// Avatar pie menu
//---------------------------------------------------------------------------
void handle_follow(void *userdata)
{
	// follow a given avatar by ID
	LLViewerObject* objectp = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
	if (objectp)
	{
		gAgent.startFollowPilot(objectp->getID());
	}
}

class LLObjectEnableMute : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerObject* object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		bool new_value = (object != NULL);
		if (new_value)
		{
			LLVOAvatar* avatar = find_avatar_from_object(object); 
			if (avatar)
			{
				// It's an avatar
				LLNameValue *lastname = avatar->getNVPair("LastName");
				BOOL is_linden = lastname && !LLStringUtil::compareStrings(lastname->getString(), "Linden");
				BOOL is_self = avatar->isSelf();
				new_value = !is_linden && !is_self;
// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
				new_value &= (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES));
// [/RLVa:KB]
			}
		}
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLObjectMute : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerObject* object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		if (!object) return true;
		
		LLUUID id;
		std::string name;
		LLMute::EType type;
		LLVOAvatar* avatar = find_avatar_from_object(object); 
		if (avatar)
		{
// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e) | Added: RLVa-1.0.0e
			if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
			{
				return true;	// Fallback code [see LLObjectEnableMute::handleEvent()]
			}
// [/RLVa:KB]
			id = avatar->getID();

			LLNameValue *firstname = avatar->getNVPair("FirstName");
			LLNameValue *lastname = avatar->getNVPair("LastName");
			if (firstname && lastname)
			{
				name = firstname->getString();
				name += " ";
				name += lastname->getString();
			}
			
			type = LLMute::AGENT;
		}
		else
		{
			// it's an object
			id = object->getID();

			LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode();
			if (node)
			{
				name = node->mName;
			}
			
			type = LLMute::OBJECT;
		}
		
		LLMute mute(id, name, type);
		if (LLMuteList::getInstance()->isMuted(mute.mID, mute.mName))
		{
			LLMuteList::getInstance()->remove(mute);
		}
		else
		{
			if( LLMute::AGENT == type )
			{
				LLMuteList::getInstance()->addMuteAgentConfirm(mute);
			}
			else
			{
				// must be an object.
				LLMuteList::getInstance()->addMuteObjectConfirm(mute);
			}
		}
		return true;
	}
};

class LLObjectEnableCopyUUID : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerObject* object = LLSelectMgr::getInstance()->getSelection()->getFirstObject();
		bool new_value = (object != NULL);
        
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLObjectCopyUUID : public view_listener_t
{
    bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
    {  
		LLViewerObject* object = LLSelectMgr::getInstance()->getSelection()->getFirstObject();
        if (!object) return true;

        LLUUID id = object->getID();

        char buffer[UUID_STR_LENGTH];
        id.toString(buffer);

        
		gViewerWindow->mWindow->copyTextToClipboard(utf8str_to_wstring(buffer));

        LLSelectMgr::getInstance()->deselectAll();
        return true;
    }
};


class LLObjectEnableExport : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLControlVariable* control = 
			gMenuHolder->findControl(userdata["control"].asString());

		LLViewerObject* object =
			LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();

		if((object != NULL) &&
		   (find_avatar_from_object(object) == NULL))
		{
			struct ff : public LLSelectedNodeFunctor
			{
				virtual bool apply(LLSelectNode* node)
				{
					return primbackup::check_perms( node );
				}
			} func;

			if(LLSelectMgr::getInstance()->getSelection()->applyToNodes(&func,false))
			{
				control->setValue(true);
				return true;	
			}
		}
 
		control->setValue(false);
		return true;
	}
};

class LLObjectExport : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerObject* object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		if (!object) return true;

		LLVOAvatar* avatar = find_avatar_from_object(object); 

		if (!avatar)
		{
			primbackup::getInstance()->pre_export_object();
		}

		return true;
	}
};


class LLObjectEnableImport : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gMenuHolder->findControl(userdata["control"].asString())->setValue(TRUE);
		return true;
	}
};

class LLObjectImport : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		primbackup::getInstance()->import_object(FALSE);
		return true;
	}
};

class LLObjectImportUpload : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		primbackup::getInstance()->import_object(TRUE);
		return true;
	}
};

bool handle_go_to()
{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
	if ( (rlv_handler_t::isEnabled()) && gAgent.forwardGrabbed() && (gRlvHandler.hasLockedAttachment()) )
	{
		return true;
	}
// [/RLVa:KB]

	if (gSavedSettings.getBOOL("DoubleClickTeleport"))
	{
		gViewerWindow->alertXml("ConfirmDoubleClickTP", handle_go_to_callback, (void*)LLToolPie::getInstance());
	}
	else if (gSavedSettings.getBOOL("DoubleClickAutoPilot"))
	{
		gViewerWindow->alertXml("ConfirmAutoPilot", handle_go_to_callback, (void*)LLToolPie::getInstance());
	}
	return true;
}

//static
void handle_go_to_callback(S32 option, void *userdata)
{
	if (option == 0)
	{
		LLToolPie* pie = (LLToolPie*)userdata;

		// JAMESDEBUG try simulator autopilot
		std::vector<std::string> strings;
		std::string val;
		LLVector3d pos = pie->getPick().mPosGlobal;
		if (gSavedSettings.getBOOL("DoubleClickTeleport"))
		{
			LLVector3d hips_offset(0.0f, 0.0f, 1.2f);
			gAgent.setControlFlags(AGENT_CONTROL_STAND_UP);
			gAgent.teleportViaLocation(pos + hips_offset);
		}
		else
		{
			// JAMESDEBUG try simulator autopilot
			std::vector<std::string> strings;
			std::string val;
			val = llformat("%g", pos.mdV[VX]);
			strings.push_back(val);
			val = llformat("%g", pos.mdV[VY]);
			strings.push_back(val);
			val = llformat("%g", pos.mdV[VZ]);
			strings.push_back(val);
			send_generic_message("autopilot", strings);

			LLViewerParcelMgr::getInstance()->deselectLand();

			if (gAgent.getAvatarObject() && !gSavedSettings.getBOOL("AutoPilotLocksCamera"))
			{
				gAgent.setFocusGlobal(gAgent.getFocusTargetGlobal(), gAgent.getAvatarObject()->getID());
			}
			else 
			{
				// Snap camera back to behind avatar
				gAgent.setFocusOnAvatar(TRUE, ANIMATE);
			}

			// Could be first use
			LLFirstUse::useGoTo();
		}
	}
}

class LLGoToObject : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		return handle_go_to();
	}
};

//---------------------------------------------------------------------------
// Parcel freeze, eject, etc.
//---------------------------------------------------------------------------
void callback_freeze(S32 option, void* data)
{
	LLUUID* avatar_id = (LLUUID*) data;

	if (0 == option || 1 == option)
	{
		U32 flags = 0x0;
		if (1 == option)
		{
			// unfreeze
			flags |= 0x1;
		}

		LLMessageSystem* msg = gMessageSystem;
		LLViewerObject* avatar = gObjectList.findObject(*avatar_id);

		if (avatar)
		{
			msg->newMessage("FreezeUser");
			msg->nextBlock("AgentData");
			msg->addUUID("AgentID", gAgent.getID());
			msg->addUUID("SessionID", gAgent.getSessionID());
			msg->nextBlock("Data");
			msg->addUUID("TargetID", *avatar_id );
			msg->addU32("Flags", flags );
			msg->sendReliable( avatar->getRegion()->getHost() );
		}
	}

	delete avatar_id;
	avatar_id = NULL;
}

class LLAvatarFreeze : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar* avatar = find_avatar_from_object( LLSelectMgr::getInstance()->getSelection()->getPrimaryObject() );
		if( avatar )
		{
			LLUUID* avatar_id = new LLUUID( avatar->getID() );
			std::string fullname = avatar->getFullname();
// [RLVa:KB] - Version: 1.22.11 | Checked: 2009-07-08 (RLVa-1.0.0e)
			if ( (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES)) && (!fullname.empty()) )
			{
				fullname = gRlvHandler.getAnonym(fullname);
			}
// [/RLVa:KB]

			if (!fullname.empty())
			{
				LLStringUtil::format_map_t args;
				args["[AVATAR_NAME]"] = fullname;
				gViewerWindow->alertXml("FreezeAvatarFullname",
							args,
							callback_freeze,
							(void*)avatar_id);
			}
			else
			{
				gViewerWindow->alertXml("FreezeAvatar",
							callback_freeze,
							(void*)avatar_id);
			}
		}
		return true;
	}
};

class LLAvatarVisibleDebug : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		//bool new_value = gAgent.isGodlike();
// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
		// TODO-RLVa: can you actually use this to cheat anything?
		bool new_value = gAgent.isGodlike() && (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES));
// [/RLVa:KB]
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLAvatarEnableDebug : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gAgent.isGodlike();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLAvatarDebug : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar* avatar = find_avatar_from_object( LLSelectMgr::getInstance()->getSelection()->getPrimaryObject() );
		if( avatar )
		{
			avatar->dumpLocalTextures();
			llinfos << "Dumping temporary asset data to simulator logs for avatar " << avatar->getID() << llendl;
			std::vector<std::string> strings;
			strings.push_back(avatar->getID().asString());
			LLUUID invoice;
			send_generic_message("dumptempassetdata", strings, invoice);
			LLFloaterAvatarTextures::show( avatar->getID() );
		}
		return true;
	}
};

struct MenuCallbackData
{
	bool ban_enabled;
	LLUUID avatar_id;
};

void callback_eject(S32 option, void* data)
{
	MenuCallbackData *callback_data = (MenuCallbackData*)data;
	if (!callback_data)
	{
		return;
	}
	if (2 == option)
	{
		// Cancle button.
		return;
	}
	LLUUID avatar_id = callback_data->avatar_id;
	bool ban_enabled = callback_data->ban_enabled;

	if (0 == option)
	{
		// Eject button
		LLMessageSystem* msg = gMessageSystem;
		LLViewerObject* avatar = gObjectList.findObject(avatar_id);

		if (avatar)
		{
			U32 flags = 0x0;
			msg->newMessage("EjectUser");
			msg->nextBlock("AgentData");
			msg->addUUID("AgentID", gAgent.getID() );
			msg->addUUID("SessionID", gAgent.getSessionID() );
			msg->nextBlock("Data");
			msg->addUUID("TargetID", avatar_id );
			msg->addU32("Flags", flags );
			msg->sendReliable( avatar->getRegion()->getHost() );
		}
	}
	else if (ban_enabled)
	{
		// This is tricky. It is similar to say if it is not an 'Eject' button,
		// and it is also not an 'Cancle' button, and ban_enabled==ture, 
		// it should be the 'Eject and Ban' button.
		LLMessageSystem* msg = gMessageSystem;
		LLViewerObject* avatar = gObjectList.findObject(avatar_id);

		if (avatar)
		{
			U32 flags = 0x1;
			msg->newMessage("EjectUser");
			msg->nextBlock("AgentData");
			msg->addUUID("AgentID", gAgent.getID() );
			msg->addUUID("SessionID", gAgent.getSessionID() );
			msg->nextBlock("Data");
			msg->addUUID("TargetID", avatar_id );
			msg->addU32("Flags", flags );
			msg->sendReliable( avatar->getRegion()->getHost() );
		}
	}


	delete callback_data;
	callback_data = NULL;
}

class LLAvatarEject : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar* avatar = find_avatar_from_object( LLSelectMgr::getInstance()->getSelection()->getPrimaryObject() );
		if( avatar )
		{
			MenuCallbackData *data = new MenuCallbackData;
			(*data).avatar_id = avatar->getID();
			std::string fullname = avatar->getFullname();
// [RLVa:KB] - Version: 1.22.11 | Checked: 2009-07-08 (RLVa-1.0.0e)
			if ( (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES)) && (!fullname.empty()) )
			{
				fullname = gRlvHandler.getAnonym(fullname);
			}
// [/RLVa:KB]

			const LLVector3d& pos = avatar->getPositionGlobal();
			LLParcel* parcel = LLViewerParcelMgr::getInstance()->selectParcelAt(pos)->getParcel();
			
			if (LLViewerParcelMgr::getInstance()->isParcelOwnedByAgent(parcel,GP_LAND_MANAGE_BANNED))
			{
				(*data).ban_enabled = true;
				if (!fullname.empty())
				{
					LLStringUtil::format_map_t args;
					args["[AVATAR_NAME]"] = fullname;
					gViewerWindow->alertXml("EjectAvatarFullname",
						args,
						callback_eject,
						(void*)data);
				}
				else
				{
					gViewerWindow->alertXml("EjectAvatar",
						callback_eject,
						(void*)data);
				}
			}
			else
			{
				(*data).ban_enabled = false;
				if (!fullname.empty())
				{
					LLStringUtil::format_map_t args;
					args["[AVATAR_NAME]"] = fullname;
					gViewerWindow->alertXml("EjectAvatarFullnameNoBan",
						args,
						callback_eject,
						(void*)data);
				}
				else
				{
					gViewerWindow->alertXml("EjectAvatarNoBan",
						callback_eject,
						(void*)data);
				}
			}
		}
		return true;
	}
};

class LLAvatarEnableFreezeEject : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar* avatar = find_avatar_from_object( LLSelectMgr::getInstance()->getSelection()->getPrimaryObject() );
		bool new_value = (avatar != NULL);

		if (new_value)
		{
			const LLVector3& pos = avatar->getPositionRegion();
			const LLVector3d& pos_global = avatar->getPositionGlobal();
			LLParcel* parcel = LLViewerParcelMgr::getInstance()->selectParcelAt(pos_global)->getParcel();
			LLViewerRegion* region = avatar->getRegion();
			new_value = (region != NULL);
						
			if (new_value)
			{
				new_value = region->isOwnedSelf(pos);
				if (!new_value || region->isOwnedGroup(pos))
				{
					new_value = LLViewerParcelMgr::getInstance()->isParcelOwnedByAgent(parcel,GP_LAND_ADMIN);
				}
			}
		}

		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLAvatarGiveCard : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
		{
			return true;
		}
// [/RLVa:KB]

		llinfos << "handle_give_card()" << llendl;
		LLViewerObject* dest = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		if(dest && dest->isAvatar())
		{
			bool found_name = false;
			LLStringUtil::format_map_t args;
			LLNameValue* nvfirst = dest->getNVPair("FirstName");
			LLNameValue* nvlast = dest->getNVPair("LastName");
			if(nvfirst && nvlast)
			{
				args["[FIRST]"] = nvfirst->getString();
				args["[LAST]"] = nvlast->getString();
				found_name = true;
			}
			LLViewerRegion* region = dest->getRegion();
			LLHost dest_host;
			if(region)
			{
				dest_host = region->getHost();
			}
			if(found_name && dest_host.isOk())
			{
				LLMessageSystem* msg = gMessageSystem;
				msg->newMessage("OfferCallingCard");
				msg->nextBlockFast(_PREHASH_AgentData);
				msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
				msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
				msg->nextBlockFast(_PREHASH_AgentBlock);
				msg->addUUIDFast(_PREHASH_DestID, dest->getID());
				LLUUID transaction_id;
				transaction_id.generate();
				msg->addUUIDFast(_PREHASH_TransactionID, transaction_id);
				msg->sendReliable(dest_host);
				LLNotifyBox::showXml("OfferedCard", args);
			}
			else
			{
				gViewerWindow->alertXml("CantOfferCallingCard", args);
			}
		}
		return true;
	}
};



void login_done(S32 which, void *user)
{
	llinfos << "Login done " << which << llendl;

	LLPanelLogin::close();
}


void callback_leave_group(S32 option, void *userdata)
{
	if (option == 0)
	{
		LLMessageSystem *msg = gMessageSystem;

		msg->newMessageFast(_PREHASH_LeaveGroupRequest);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		msg->nextBlockFast(_PREHASH_GroupData);
		msg->addUUIDFast(_PREHASH_GroupID, gAgent.mGroupID );
		gAgent.sendReliableMessage();
	}
}

void handle_leave_group(void *)
{
	if (gAgent.getGroupID() != LLUUID::null)
	{
		LLStringUtil::format_map_t args;
		args["[GROUP]"] = gAgent.mGroupName;
		gViewerWindow->alertXml("GroupLeaveConfirmMember", args, callback_leave_group);
	}
}

void append_aggregate(std::string& string, const LLAggregatePermissions& ag_perm, PermissionBit bit, const char* txt)
{
	LLAggregatePermissions::EValue val = ag_perm.getValue(bit);
	std::string buffer;
	switch(val)
	{
	  case LLAggregatePermissions::AP_NONE:
		buffer = llformat( "* %s None\n", txt);
		break;
	  case LLAggregatePermissions::AP_SOME:
		buffer = llformat( "* %s Some\n", txt);
		break;
	  case LLAggregatePermissions::AP_ALL:
		buffer = llformat( "* %s All\n", txt);
		break;
	  case LLAggregatePermissions::AP_EMPTY:
	  default:
		break;
	}
	string.append(buffer);
}

BOOL enable_buy(void*)
{
    // In order to buy, there must only be 1 purchaseable object in
    // the selection manger.
	if(LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() != 1) return FALSE;
    LLViewerObject* obj = NULL;
    LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode();
	if(node)
    {
        obj = node->getObject();
        if(!obj) return FALSE;

		if(node->mSaleInfo.isForSale() && node->mPermissions->getMaskOwner() & PERM_TRANSFER &&
			(node->mPermissions->getMaskOwner() & PERM_COPY || node->mSaleInfo.getSaleType() != LLSaleInfo::FS_COPY))
		{
			if(obj->permAnyOwner()) return TRUE;
		}
    }
	return FALSE;
}

class LLObjectEnableBuy : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = enable_buy(NULL);
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

// Note: This will only work if the selected object's data has been
// received by the viewer and cached in the selection manager.
void handle_buy_object(LLSaleInfo sale_info)
{
	if(!LLSelectMgr::getInstance()->selectGetAllRootsValid())
	{
		LLNotifyBox::showXml("UnableToBuyWhileDownloading");
		return;
	}

	LLUUID owner_id;
	std::string owner_name;
	BOOL owners_identical = LLSelectMgr::getInstance()->selectGetOwner(owner_id, owner_name);
	if (!owners_identical)
	{
		LLNotifyBox::showXml("CannotBuyObjectsFromDifferentOwners");
		return;
	}

	LLPermissions perm;
	BOOL valid = LLSelectMgr::getInstance()->selectGetPermissions(perm);
	LLAggregatePermissions ag_perm;
	valid &= LLSelectMgr::getInstance()->selectGetAggregatePermissions(ag_perm);
	if(!valid || !sale_info.isForSale() || !perm.allowTransferTo(gAgent.getID()))
	{
		LLNotifyBox::showXml("ObjectNotForSale");
		return;
	}

	S32 price = sale_info.getSalePrice();
	
	if (price > 0 && price > gStatusBar->getBalance())
	{
		LLFloaterBuyCurrency::buyCurrency("This object costs", price);
		return;
	}

	LLFloaterBuy::show(sale_info);
}


void handle_buy_contents(LLSaleInfo sale_info)
{
	LLFloaterBuyContents::show(sale_info);
}

void handle_region_dump_temp_asset_data(void*)
{
	llinfos << "Dumping temporary asset data to simulator logs" << llendl;
	std::vector<std::string> strings;
	LLUUID invoice;
	send_generic_message("dumptempassetdata", strings, invoice);
}

void handle_region_clear_temp_asset_data(void*)
{
	llinfos << "Clearing temporary asset data" << llendl;
	std::vector<std::string> strings;
	LLUUID invoice;
	send_generic_message("cleartempassetdata", strings, invoice);
}

void handle_region_dump_settings(void*)
{
	LLViewerRegion* regionp = gAgent.getRegion();
	if (regionp)
	{
		llinfos << "Damage:    " << (regionp->getAllowDamage() ? "on" : "off") << llendl;
		llinfos << "Landmark:  " << (regionp->getAllowLandmark() ? "on" : "off") << llendl;
		llinfos << "SetHome:   " << (regionp->getAllowSetHome() ? "on" : "off") << llendl;
		llinfos << "ResetHome: " << (regionp->getResetHomeOnTeleport() ? "on" : "off") << llendl;
		llinfos << "SunFixed:  " << (regionp->getSunFixed() ? "on" : "off") << llendl;
		llinfos << "BlockFly:  " << (regionp->getBlockFly() ? "on" : "off") << llendl;
		llinfos << "AllowP2P:  " << (regionp->getAllowDirectTeleport() ? "on" : "off") << llendl;
		llinfos << "Water:     " << (regionp->getWaterHeight()) << llendl;
	}
}

void handle_dump_group_info(void *)
{
	llinfos << "group   " << gAgent.mGroupName << llendl;
	llinfos << "ID      " << gAgent.mGroupID << llendl;
	llinfos << "powers " << gAgent.mGroupPowers << llendl;
	llinfos << "title   " << gAgent.mGroupTitle << llendl;
	//llinfos << "insig   " << gAgent.mGroupInsigniaID << llendl;
}

void handle_dump_capabilities_info(void *)
{
	LLViewerRegion* regionp = gAgent.getRegion();
	if (regionp)
	{
		regionp->logActiveCapabilities();
	}
}

void handle_dump_region_object_cache(void*)
{
	LLViewerRegion* regionp = gAgent.getRegion();
	if (regionp)
	{
		regionp->dumpCache();
	}
}

void handle_dump_focus(void *)
{
	LLUICtrl *ctrl = dynamic_cast<LLUICtrl*>(gFocusMgr.getKeyboardFocus());

	llinfos << "Keyboard focus " << (ctrl ? ctrl->getName() : "(none)") << llendl;
}

class LLSelfStandUp : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT))
		{
			return true;
		}
// [/RLVa:KB]

		gAgent.setControlFlags(AGENT_CONTROL_STAND_UP);
		return true;
	}
};

class LLSelfEnableStandUp : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
//		bool new_value = gAgent.getAvatarObject() && gAgent.getAvatarObject()->mIsSitting;
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
		bool new_value = gAgent.getAvatarObject() && gAgent.getAvatarObject()->mIsSitting && !gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT);
// [/RLVa:KB]
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

BOOL check_admin_override(void*)
{
	return gAgent.getAdminOverride();
}

void handle_admin_override_toggle(void*)
{
	gAgent.setAdminOverride(!gAgent.getAdminOverride());

	// The above may have affected which debug menus are visible
	show_debug_menus();
}

void handle_god_mode(void*)
{
	gAgent.requestEnterGodMode();
}

void handle_leave_god_mode(void*)
{
	gAgent.requestLeaveGodMode();
}

void set_god_level(U8 god_level)
{
	U8 old_god_level = gAgent.getGodLevel();
	gAgent.setGodLevel( god_level );
	gIMMgr->refresh();
	LLViewerParcelMgr::getInstance()->notifyObservers();

	// Some classifieds change visibility on god mode
	LLFloaterDirectory::requestClassifieds();

	// God mode changes sim visibility
	LLWorldMap::getInstance()->reset();
	LLWorldMap::getInstance()->setCurrentLayer(0);

	// inventory in items may change in god mode
	gObjectList.dirtyAllObjectInventory();

    if(gViewerWindow)
    {
        gViewerWindow->setMenuBackgroundColor(god_level > GOD_NOT,
            LLViewerLogin::getInstance()->isInProductionGrid());
    }

    LLStringUtil::format_map_t args;
	if(god_level > GOD_NOT)
	{
		args["[LEVEL]"] = llformat("%d",(S32)god_level);
		LLNotifyBox::showXml("EnteringGodMode", args);
	}
	else
	{
		args["[LEVEL]"] = llformat("%d",(S32)old_god_level);
		LLNotifyBox::showXml("LeavingGodMode", args);
	}


	// changing god-level can affect which menus we see
	show_debug_menus();
}

#ifdef TOGGLE_HACKED_GODLIKE_VIEWER
void handle_toggle_hacked_godmode(void*)
{
	gHackGodmode = !gHackGodmode;
	set_god_level(gHackGodmode ? GOD_MAINTENANCE : GOD_NOT);
}

BOOL check_toggle_hacked_godmode(void*)
{
	return gHackGodmode;
}
#endif

void process_grant_godlike_powers(LLMessageSystem* msg, void**)
{
	LLUUID agent_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id);
	LLUUID session_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_SessionID, session_id);
	if((agent_id == gAgent.getID()) && (session_id == gAgent.getSessionID()))
	{
		U8 god_level;
		msg->getU8Fast(_PREHASH_GrantData, _PREHASH_GodLevel, god_level);
		set_god_level(god_level);
	}
	else
	{
		llwarns << "Grant godlike for wrong agent " << agent_id << llendl;
	}
}

/*
class LLHaveCallingcard : public LLInventoryCollectFunctor
{
public:
	LLHaveCallingcard(const LLUUID& agent_id);
	virtual ~LLHaveCallingcard() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
	BOOL isThere() const { return mIsThere;}
protected:
	LLUUID mID;
	BOOL mIsThere;
};

LLHaveCallingcard::LLHaveCallingcard(const LLUUID& agent_id) :
	mID(agent_id),
	mIsThere(FALSE)
{
}

bool LLHaveCallingcard::operator()(LLInventoryCategory* cat,
								   LLInventoryItem* item)
{
	if(item)
	{
		if((item->getType() == LLAssetType::AT_CALLINGCARD)
		   && (item->getCreatorUUID() == mID))
		{
			mIsThere = TRUE;
		}
	}
	return FALSE;
}
*/

BOOL is_agent_friend(const LLUUID& agent_id)
{
	return (LLAvatarTracker::instance().getBuddyInfo(agent_id) != NULL);
}

BOOL is_agent_mappable(const LLUUID& agent_id)
{
	return (is_agent_friend(agent_id) &&
		LLAvatarTracker::instance().getBuddyInfo(agent_id)->isOnline() &&
		LLAvatarTracker::instance().getBuddyInfo(agent_id)->isRightGrantedFrom(LLRelationship::GRANT_MAP_LOCATION)
		);
}

// Enable a menu item when you have someone's card.
/*
BOOL enable_have_card(void *userdata)
{
	LLUUID* avatar_id = (LLUUID *)userdata;
	if (gAgent.isGodlike())
	{
		return TRUE;
	}
	else if(avatar_id)
	{
		return is_agent_friend(*avatar_id);
	}
	else
	{
		return FALSE;
	}
}
*/

// Enable a menu item when you don't have someone's card.
class LLAvatarEnableAddFriend : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar* avatar = find_avatar_from_object(LLSelectMgr::getInstance()->getSelection()->getPrimaryObject());
		bool new_value = avatar && !is_agent_friend(avatar->getID());

// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
		new_value &= (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES));
// [/RLVa:KB]

		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

void request_friendship(const LLUUID& dest_id)
{
	LLViewerObject* dest = gObjectList.findObject(dest_id);
	if(dest && dest->isAvatar())
	{
		std::string fullname;
		LLStringUtil::format_map_t args;
		LLNameValue* nvfirst = dest->getNVPair("FirstName");
		LLNameValue* nvlast = dest->getNVPair("LastName");
		if(nvfirst && nvlast)
		{
			args["[FIRST]"] = nvfirst->getString();
			args["[LAST]"] = nvlast->getString();
			fullname = nvfirst->getString();
			fullname += " ";
			fullname += nvlast->getString();
		}
		if (!fullname.empty())
		{
			LLPanelFriends::requestFriendshipDialog(dest_id, fullname);
		}
		else
		{
			gViewerWindow->alertXml("CantOfferFriendship");
		}
	}
}


class LLEditEnableCustomizeAvatar : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar* avatar = gAgent.getAvatarObject();

		bool enabled = ((avatar && avatar->isFullyLoaded()) &&
				   (gAgent.getWearablesLoaded()));

		gMenuHolder->findControl(userdata["control"].asString())->setValue(enabled);
		return true;
	}
};

// only works on pie menu
bool handle_sit_or_stand()
{
	LLPickInfo pick = LLToolPie::getInstance()->getPick();
	LLViewerObject *object = pick.getObject();;
	if (!object || pick.mPickType == LLPickInfo::PICK_FLORA)
	{
		return true;
	}

// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0c)
	if ( (rlv_handler_t::isEnabled()) && 
		 ( ((gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) && (gAgent.getAvatarObject()) && (gAgent.getAvatarObject()->mIsSitting)) ||
		   (gRlvHandler.hasBehaviour(RLV_BHVR_SIT)) ) )
	{
		return true;
	}
// [/RLVa:KB]

	if (sitting_on_selection())
	{
		gAgent.setControlFlags(AGENT_CONTROL_STAND_UP);
		return true;
	}

	// get object selection offset 

	if (object && object->getPCode() == LL_PCODE_VOLUME)
	{
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g) | Modified: RLVa-0.2.0g
		if ( (rlv_handler_t::isEnabled()) && 
			 ((gRlvHandler.hasBehaviour(RLV_BHVR_SITTP)) || (gRlvHandler.hasBehaviour(RLV_BHVR_FARTOUCH))) &&
			 (dist_vec_squared(gAgent.getPositionGlobal(), object->getPositionGlobal() + LLVector3d(pick.mObjectOffset)) > 1.5f * 1.5f) )
		{
			return true;	// Don't allow sitting farther away than 1.5m under @sittp=n or @fartouch=n
		}
// [/RLVa:KB]

		gMessageSystem->newMessageFast(_PREHASH_AgentRequestSit);
		gMessageSystem->nextBlockFast(_PREHASH_AgentData);
		gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		gMessageSystem->nextBlockFast(_PREHASH_TargetObject);
		gMessageSystem->addUUIDFast(_PREHASH_TargetID, object->mID);
		gMessageSystem->addVector3Fast(_PREHASH_Offset, pick.mObjectOffset);

		object->getRegion()->sendReliableMessage();
	}
	return true;
}

class LLObjectSitOrStand : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		return handle_sit_or_stand();
	}
};

void near_sit_down_point(BOOL success, void *)
{
	if (success)
	{
		gAgent.setFlying(FALSE);
		gAgent.setControlFlags(AGENT_CONTROL_SIT_ON_GROUND);

		// Might be first sit
		LLFirstUse::useSit();
	}
}

class LLLandSit : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT))
		{
			return true;
		}
// [/RLVa:KB]

		gAgent.setControlFlags(AGENT_CONTROL_STAND_UP);
		LLViewerParcelMgr::getInstance()->deselectLand();

		LLVector3d posGlobal = LLToolPie::getInstance()->getPick().mPosGlobal;
		
		LLQuaternion target_rot;
		if (gAgent.getAvatarObject())
		{
			target_rot = gAgent.getAvatarObject()->getRotation();
		}
		else
		{
			target_rot = gAgent.getFrameAgent().getQuaternion();
		}
		gAgent.startAutoPilotGlobal(posGlobal, "Sit", &target_rot, near_sit_down_point, NULL, 0.7f);
		return true;
	}
};

void show_permissions_control(void*)
{
	LLFloaterPermissionsMgr* floaterp = LLFloaterPermissionsMgr::show();
	floaterp->mPermissions->addPermissionsData("foo1", LLUUID::null, 0);
	floaterp->mPermissions->addPermissionsData("foo2", LLUUID::null, 0);
	floaterp->mPermissions->addPermissionsData("foo3", LLUUID::null, 0);
}


class LLCreateLandmarkCallback : public LLInventoryCallback
{
public:
	/*virtual*/ void fire(const LLUUID& inv_item)
	{
		llinfos << "Created landmark with inventory id " << inv_item
			<< llendl;
	}
};

void reload_ui(void *)
{
	LLUICtrlFactory::getInstance()->rebuild();
}

class LLWorldFly : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gAgent.toggleFlying();
		return true;
	}
};

class LLWorldEnableFly : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		BOOL sitting = FALSE;
		if (gAgent.getAvatarObject())
		{
			sitting = gAgent.getAvatarObject()->mIsSitting;
		}
		gMenuHolder->findControl(userdata["control"].asString())->setValue(!sitting);
		return true;
	}
};


void handle_agent_stop_moving(void*)
{
	// stop agent
	gAgent.setControlFlags(AGENT_CONTROL_STOP);

	// cancel autopilot
	gAgent.stopAutoPilot();
}

void print_packets_lost(void*)
{
	LLWorld::getInstance()->printPacketsLost();
}


void drop_packet(void*)
{
	gMessageSystem->mPacketRing.dropPackets(1);
}


void velocity_interpolate( void* data )
{
	BOOL toggle = gSavedSettings.getBOOL("VelocityInterpolate");
	LLMessageSystem* msg = gMessageSystem;
	if ( !toggle )
	{
		msg->newMessageFast(_PREHASH_VelocityInterpolateOn);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		gAgent.sendReliableMessage();
		llinfos << "Velocity Interpolation On" << llendl;
	}
	else
	{
		msg->newMessageFast(_PREHASH_VelocityInterpolateOff);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		gAgent.sendReliableMessage();
		llinfos << "Velocity Interpolation Off" << llendl;
	}
	// BUG this is a hack because of the change in menu behavior.  The
	// old menu system would automatically change a control's value,
	// but the new LLMenuGL system doesn't know what a control
	// is. However, it's easy to distinguish between the two callers
	// because LLMenuGL passes in the name of the user data (the
	// control name) to the callback function, and the user data goes
	// unused in the old menu code. Thus, if data is not null, then we
	// need to swap the value of the control.
	if( data )
	{
		gSavedSettings.setBOOL( static_cast<char*>(data), !toggle );
	}
}


void update_fov(S32 increments)
{
	F32 old_fov = LLViewerCamera::getInstance()->getDefaultFOV();
	// for each increment, FoV is 20% bigger
	F32 new_fov = old_fov * pow(1.2f, increments);

	// cap the FoV
	new_fov = llclamp(new_fov, MIN_FIELD_OF_VIEW, MAX_FIELD_OF_VIEW);

	if (new_fov != old_fov)
	{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_AgentFOV);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		msg->addU32Fast(_PREHASH_CircuitCode, gMessageSystem->mOurCircuitCode);

		msg->nextBlockFast(_PREHASH_FOVBlock);
		msg->addU32Fast(_PREHASH_GenCounter, 0);
		msg->addF32Fast(_PREHASH_VerticalAngle, new_fov);

		gAgent.sendReliableMessage();

		// force agent to update dirty patches
		LLViewerCamera::getInstance()->setDefaultFOV(new_fov);
		LLViewerCamera::getInstance()->setView(new_fov);
	}
}

class LLViewZoomOut : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		update_fov(1);
		return true;
	}
};

class LLViewZoomIn : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		update_fov(-1);
		return true;
	}
};

class LLViewZoomDefault : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		F32 old_fov = LLViewerCamera::getInstance()->getView();
		// for each increment, FoV is 20% bigger
		F32 new_fov = DEFAULT_FIELD_OF_VIEW;

		if (new_fov != old_fov)
		{
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_AgentFOV);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->addU32Fast(_PREHASH_CircuitCode, gMessageSystem->mOurCircuitCode);
			msg->nextBlockFast(_PREHASH_FOVBlock);
			msg->addU32Fast(_PREHASH_GenCounter, 0);
			msg->addF32Fast(_PREHASH_VerticalAngle, new_fov);

			gAgent.sendReliableMessage();

			// force agent to update dirty patches
			LLViewerCamera::getInstance()->setDefaultFOV(new_fov);
			LLViewerCamera::getInstance()->setView(new_fov);
		}
		return true;
	}
};



void toggle_wind_audio(void)
{
	if (gAudiop)
	{
		gAudiop->enableWind(!(gAudiop->isWindEnabled()));
	}
}


// Callback for enablement
BOOL is_inventory_visible( void* user_data )
{
	LLInventoryView* iv = reinterpret_cast<LLInventoryView*>(user_data);
	if( iv )
	{
		return iv->getVisible();
	}
	return FALSE;
}

void handle_show_newest_map(void*)
{
	LLFloaterWorldMap::show(NULL, FALSE);
}

//-------------------------------------------------------------------
// Help menu functions
//-------------------------------------------------------------------

//
// Major mode switching
//
void reset_view_final( BOOL proceed, void* );

void handle_reset_view()
{
	if( (CAMERA_MODE_CUSTOMIZE_AVATAR == gAgent.getCameraMode()) && gFloaterCustomize )
	{
		// Show dialog box if needed.
		gFloaterCustomize->askToSaveAllIfDirty( reset_view_final, NULL );
	}
	else
	{
		reset_view_final( TRUE, NULL );
	}
}

class LLViewResetView : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_reset_view();
		return true;
	}
};

// Note: extra parameters allow this function to be called from dialog.
void reset_view_final( BOOL proceed, void* ) 
{
	if( !proceed )
	{
		return;
	}

	gAgent.resetView(TRUE, TRUE);
}

class LLViewLookAtLastChatter : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gAgent.lookAtLastChat();
		return true;
	}
};

class LLViewMouselook : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (!gAgent.cameraMouselook())
		{
			gAgent.changeCameraToMouselook();
		}
		else
		{
			gAgent.changeCameraToDefault();
		}
		return true;
	}
};

class LLViewFullscreen : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gViewerWindow->toggleFullscreenConfirm();
		return true;
	}
};

class LLEditDuplicate : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		if ( (rlv_handler_t::isEnabled()) && (gRlvHandler.hasBehaviour(RLV_BHVR_REZ)) && 
			 (LLEditMenuHandler::gEditMenuHandler == LLSelectMgr::getInstance()) )
		{
			return true;
		}
// [/RLVa:KB]

		if(LLEditMenuHandler::gEditMenuHandler)
		{
			LLEditMenuHandler::gEditMenuHandler->duplicate();
		}
		return true;
	}
};

class LLEditEnableDuplicate : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canDuplicate();
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		if ( (new_value) && (rlv_handler_t::isEnabled()) && (gRlvHandler.hasBehaviour(RLV_BHVR_REZ)) && 
			 (LLEditMenuHandler::gEditMenuHandler == LLSelectMgr::getInstance()) )
		{
			new_value = false;
		}
// [/RLVa:KB]
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};


void disabled_duplicate(void*)
{
	if (LLSelectMgr::getInstance()->getSelection()->getPrimaryObject())
	{
		LLNotifyBox::showXml("CopyFailed");
	}
}

void handle_duplicate_in_place(void*)
{
	llinfos << "handle_duplicate_in_place" << llendl;

	LLVector3 offset(0.f, 0.f, 0.f);
	LLSelectMgr::getInstance()->selectDuplicate(offset, TRUE);
}

void handle_repeat_duplicate(void*)
{
	LLSelectMgr::getInstance()->repeatDuplicate();
}

/* dead code 30-apr-2008
void handle_deed_object_to_group(void*)
{
	LLUUID group_id;
	
	LLSelectMgr::getInstance()->selectGetGroup(group_id);
	LLSelectMgr::getInstance()->sendOwner(LLUUID::null, group_id, FALSE);
	LLViewerStats::getInstance()->incStat(LLViewerStats::ST_RELEASE_COUNT);
}

BOOL enable_deed_object_to_group(void*)
{
	if(LLSelectMgr::getInstance()->getSelection()->isEmpty()) return FALSE;
	LLPermissions perm;
	LLUUID group_id;

	if (LLSelectMgr::getInstance()->selectGetGroup(group_id) &&
		gAgent.hasPowerInGroup(group_id, GP_OBJECT_DEED) &&
		LLSelectMgr::getInstance()->selectGetPermissions(perm) &&
		perm.deedToGroup(gAgent.getID(), group_id))
	{
		return TRUE;
	}
	return FALSE;
}

*/


/*
 * No longer able to support viewer side manipulations in this way
 *
void god_force_inv_owner_permissive(LLViewerObject* object,
									InventoryObjectList* inventory,
									S32 serial_num,
									void*)
{
	typedef std::vector<LLPointer<LLViewerInventoryItem> > item_array_t;
	item_array_t items;

	InventoryObjectList::const_iterator inv_it = inventory->begin();
	InventoryObjectList::const_iterator inv_end = inventory->end();
	for ( ; inv_it != inv_end; ++inv_it)
	{
		if(((*inv_it)->getType() != LLAssetType::AT_CATEGORY)
		   && ((*inv_it)->getType() != LLAssetType::AT_ROOT_CATEGORY))
		{
			LLInventoryObject* obj = *inv_it;
			LLPointer<LLViewerInventoryItem> new_item = new LLViewerInventoryItem((LLViewerInventoryItem*)obj);
			LLPermissions perm(new_item->getPermissions());
			perm.setMaskBase(PERM_ALL);
			perm.setMaskOwner(PERM_ALL);
			new_item->setPermissions(perm);
			items.push_back(new_item);
		}
	}
	item_array_t::iterator end = items.end();
	item_array_t::iterator it;
	for(it = items.begin(); it != end; ++it)
	{
		// since we have the inventory item in the callback, it should not
		// invalidate iteration through the selection manager.
		object->updateInventory((*it), TASK_INVENTORY_ITEM_KEY, false);
	}
}
*/

void handle_object_owner_permissive(void*)
{
	// only send this if they're a god.
	if(gAgent.isGodlike())
	{
		// do the objects.
		LLSelectMgr::getInstance()->selectionSetObjectPermissions(PERM_BASE, TRUE, PERM_ALL, TRUE);
		LLSelectMgr::getInstance()->selectionSetObjectPermissions(PERM_OWNER, TRUE, PERM_ALL, TRUE);
	}
}

void handle_object_owner_self(void*)
{
	// only send this if they're a god.
	if(gAgent.isGodlike())
	{
		LLSelectMgr::getInstance()->sendOwner(gAgent.getID(), gAgent.getGroupID(), TRUE);
	}
}

// Shortcut to set owner permissions to not editable.
void handle_object_lock(void*)
{
	LLSelectMgr::getInstance()->selectionSetObjectPermissions(PERM_OWNER, FALSE, PERM_MODIFY);
}

void handle_object_asset_ids(void*)
{
	// only send this if they're a god.
	if (gAgent.isGodlike())
	{
		LLSelectMgr::getInstance()->sendGodlikeRequest("objectinfo", "assetids");
	}
}

void handle_force_parcel_owner_to_me(void*)
{
	LLViewerParcelMgr::getInstance()->sendParcelGodForceOwner( gAgent.getID() );
}

void handle_force_parcel_to_content(void*)
{
	LLViewerParcelMgr::getInstance()->sendParcelGodForceToContent();
}

void handle_claim_public_land(void*)
{
	if (LLViewerParcelMgr::getInstance()->getSelectionRegion() != gAgent.getRegion())
	{
		LLNotifyBox::showXml("ClaimPublicLand");
		return;
	}

	LLVector3d west_south_global;
	LLVector3d east_north_global;
	LLViewerParcelMgr::getInstance()->getSelection(west_south_global, east_north_global);
	LLVector3 west_south = gAgent.getPosAgentFromGlobal(west_south_global);
	LLVector3 east_north = gAgent.getPosAgentFromGlobal(east_north_global);

	LLMessageSystem* msg = gMessageSystem;
	msg->newMessage("GodlikeMessage");
	msg->nextBlock("AgentData");
	msg->addUUID("AgentID", gAgent.getID());
	msg->addUUID("SessionID", gAgent.getSessionID());
	msg->addUUIDFast(_PREHASH_TransactionID, LLUUID::null); //not used
	msg->nextBlock("MethodData");
	msg->addString("Method", "claimpublicland");
	msg->addUUID("Invoice", LLUUID::null);
	std::string buffer;
	buffer = llformat( "%f", west_south.mV[VX]);
	msg->nextBlock("ParamList");
	msg->addString("Parameter", buffer);
	buffer = llformat( "%f", west_south.mV[VY]);
	msg->nextBlock("ParamList");
	msg->addString("Parameter", buffer);
	buffer = llformat( "%f", east_north.mV[VX]);
	msg->nextBlock("ParamList");
	msg->addString("Parameter", buffer);
	buffer = llformat( "%f", east_north.mV[VY]);
	msg->nextBlock("ParamList");
	msg->addString("Parameter", buffer);
	gAgent.sendReliableMessage();
}

void handle_god_request_havok(void *)
{
	if (gAgent.isGodlike())
	{
		LLSelectMgr::getInstance()->sendGodlikeRequest("havok", "infoverbose");
	}
}

//void handle_god_request_foo(void *)
//{
//	if (gAgent.isGodlike())
//	{
//		LLSelectMgr::getInstance()->sendGodlikeRequest(GOD_WANTS_FOO);
//	}
//}

//void handle_god_request_terrain_save(void *)
//{
//	if (gAgent.isGodlike())
//	{
//		LLSelectMgr::getInstance()->sendGodlikeRequest("terrain", "save");
//	}
//}

//void handle_god_request_terrain_load(void *)
//{
//	if (gAgent.isGodlike())
//	{
//		LLSelectMgr::getInstance()->sendGodlikeRequest("terrain", "load");
//	}
//}


// HACK for easily testing new avatar geometry
void handle_god_request_avatar_geometry(void *)
{
	if (gAgent.isGodlike())
	{
		LLSelectMgr::getInstance()->sendGodlikeRequest("avatar toggle", NULL);
	}
}


void handle_show_overlay_title(void*)
{
	gShowOverlayTitle = !gShowOverlayTitle;
	gSavedSettings.setBOOL("ShowOverlayTitle", gShowOverlayTitle);
}

void derez_objects(EDeRezDestination dest, const LLUUID& dest_id)
{
	if(gAgent.cameraMouselook())
	{
		gAgent.changeCameraToDefault();
	}
	//gInventoryView->setPanelOpen(TRUE);

	std::string error;
	LLDynamicArray<LLViewerObject*> derez_objects;
	
	// Check conditions that we can't deal with, building a list of
	// everything that we'll actually be derezzing.
	LLViewerRegion* first_region = NULL;
	for (LLObjectSelection::valid_root_iterator iter = LLSelectMgr::getInstance()->getSelection()->valid_root_begin();
		 iter != LLSelectMgr::getInstance()->getSelection()->valid_root_end(); iter++)
	{
		LLSelectNode* node = *iter;
		LLViewerObject* object = node->getObject();
		LLViewerRegion* region = object->getRegion();
		if (!first_region)
		{
			first_region = region;
		}
		else
		{
			if(region != first_region)
			{
				// Derez doesn't work at all if the some of the objects
				// are in regions besides the first object selected.
				
				// ...crosses region boundaries
				error = "AcquireErrorObjectSpan";
				break;
			}
		}
		if (object->isAvatar())
		{
			// ...don't acquire avatars
			continue;
		}

		// If AssetContainers are being sent back, they will appear as 
		// boxes in the owner's inventory.
		if (object->getNVPair("AssetContainer")
			&& dest != DRD_RETURN_TO_OWNER)
		{
			// this object is an asset container, derez its contents, not it
			llwarns << "Attempt to derez deprecated AssetContainer object type not supported." << llendl;
			/*
			object->requestInventory(container_inventory_arrived, 
				(void *)(BOOL)(DRD_TAKE_INTO_AGENT_INVENTORY == dest));
			*/
			continue;
		}
		BOOL can_derez_current = FALSE;
		switch(dest)
		{
		case DRD_TAKE_INTO_AGENT_INVENTORY:
		case DRD_TRASH:
			if( (node->mPermissions->allowTransferTo(gAgent.getID()) && object->permModify())
				|| (node->allowOperationOnNode(PERM_OWNER, GP_OBJECT_MANIPULATE)) )
			{
				can_derez_current = TRUE;
			}
			break;

		case DRD_RETURN_TO_OWNER:
			can_derez_current = TRUE;
			break;

		default:
			if((node->mPermissions->allowTransferTo(gAgent.getID())
				&& object->permCopy())
			   || gAgent.isGodlike())
			{
				can_derez_current = TRUE;
			}
			break;
		}
		if(can_derez_current)
		{
			derez_objects.put(object);
		}
	}

	// This constant is based on (1200 - HEADER_SIZE) / 4 bytes per
	// root.  I lopped off a few (33) to provide a bit
	// pad. HEADER_SIZE is currently 67 bytes, most of which is UUIDs.
	// This gives us a maximum of 63500 root objects - which should
	// satisfy anybody.
	const S32 MAX_ROOTS_PER_PACKET = 250;
	const S32 MAX_PACKET_COUNT = 254;
	F32 packets = ceil((F32)derez_objects.count() / (F32)MAX_ROOTS_PER_PACKET);
	if(packets > (F32)MAX_PACKET_COUNT)
	{
		error = "AcquireErrorTooManyObjects";
	}

	if(error.empty() && derez_objects.count() > 0)
	{
		U8 d = (U8)dest;
		LLUUID tid;
		tid.generate();
		U8 packet_count = (U8)packets;
		S32 object_index = 0;
		S32 objects_in_packet = 0;
		LLMessageSystem* msg = gMessageSystem;
		for(U8 packet_number = 0;
			packet_number < packet_count;
			++packet_number)
		{
			msg->newMessageFast(_PREHASH_DeRezObject);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_AgentBlock);
			msg->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());
			msg->addU8Fast(_PREHASH_Destination, d);	
			msg->addUUIDFast(_PREHASH_DestinationID, dest_id);
			msg->addUUIDFast(_PREHASH_TransactionID, tid);
			msg->addU8Fast(_PREHASH_PacketCount, packet_count);
			msg->addU8Fast(_PREHASH_PacketNumber, packet_number);
			objects_in_packet = 0;
			while((object_index < derez_objects.count())
				  && (objects_in_packet++ < MAX_ROOTS_PER_PACKET))

			{
				LLViewerObject* object = derez_objects.get(object_index++);
				msg->nextBlockFast(_PREHASH_ObjectData);
				msg->addU32Fast(_PREHASH_ObjectLocalID, object->getLocalID());
				// VEFFECT: DerezObject
				LLHUDEffectSpiral* effectp = (LLHUDEffectSpiral*)LLHUDManager::getInstance()->createViewerEffect(LLHUDObject::LL_HUD_EFFECT_POINT, TRUE);
				effectp->setPositionGlobal(object->getPositionGlobal());
				effectp->setColor(LLColor4U(gAgent.getEffectColor()));
			}
			msg->sendReliable(first_region->getHost());
		}
		make_ui_sound("UISndObjectRezOut");

		// Busy count decremented by inventory update, so only increment
		// if will be causing an update.
		if (dest != DRD_RETURN_TO_OWNER)
		{
			gViewerWindow->getWindow()->incBusyCount();
		}
	}
	else if(!error.empty())
	{
		gViewerWindow->alertXml(error);
	}
}

class LLToolsTakeCopy : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (LLSelectMgr::getInstance()->getSelection()->isEmpty()) return true;
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b) | Modified: RLVa-1.0.0b
		// NOTE: we need to handle "Take Copy" because it will force a sim-side unsit if we're sitting on the selection, 
		//       but we do want to allow "Take Copy" under @rez=n so that's why we explicitly check for @unsit=n here
		if ( (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) && (!rlvCanDeleteOrReturn()) ) return true;
// [/RLVa:KB]

		const LLUUID& category_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_OBJECT);
		derez_objects(DRD_ACQUIRE_TO_AGENT_INVENTORY, category_id);

		return true;
	}
};


// You can return an object to its owner if it is on your land.
class LLObjectReturn : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (LLSelectMgr::getInstance()->getSelection()->isEmpty()) return true;
// [RLVa:KB] - Version: 1.22.11 | Checked: 2009-07-05 (RLVa-1.0.0b)
		if ( (rlv_handler_t::isEnabled()) && (!rlvCanDeleteOrReturn()) ) return true;
// [/RLVa:KB]
		
		mObjectSelection = LLSelectMgr::getInstance()->getEditSelection();

		gViewerWindow->alertXml("ReturnToOwner",
			onReturnToOwner,
			(void*)this);
		return true;
	}

	static void onReturnToOwner(S32 option, void* data)
	{
		LLObjectReturn* object_return = (LLObjectReturn*)data;

		if (0 == option)
		{
			// Ignore category ID for this derez destination.
			derez_objects(DRD_RETURN_TO_OWNER, LLUUID::null);
		}

		// drop reference to current selection
		object_return->mObjectSelection = NULL;
	}

protected:
	LLObjectSelectionHandle mObjectSelection;
};


// Allow return to owner if one or more of the selected items is
// over land you own.
class LLObjectEnableReturn : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
#ifdef HACKED_GODLIKE_VIEWER
		bool new_value = true;
#else
		bool new_value = false;
		if (gAgent.isGodlike())
		{
			new_value = true;
		}
		else
		{
			LLViewerRegion* region = gAgent.getRegion();
			if (region)
			{
				// Estate owners and managers can always return objects.
				if (region->canManageEstate())
				{
					new_value = true;
				}
				else
				{
					struct f : public LLSelectedObjectFunctor
					{
						virtual bool apply(LLViewerObject* obj)
						{
							return (obj->isOverAgentOwnedLand() ||
									obj->isOverGroupOwnedLand() ||
									obj->permModify());
						}
					} func;
					const bool firstonly = true;
					new_value = LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func, firstonly);
				}
			}
		}
#endif

// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		if ( (new_value) && (rlv_handler_t::isEnabled()) )
		{
			new_value = rlvCanDeleteOrReturn();
		}
// [/RLVa:KB]

		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

void force_take_copy(void*)
{
	if (LLSelectMgr::getInstance()->getSelection()->isEmpty()) return;
	const LLUUID& category_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_OBJECT);
	derez_objects(DRD_FORCE_TO_GOD_INVENTORY, category_id);
}

void handle_take()
{
	// we want to use the folder this was derezzed from if it's
	// available. Otherwise, derez to the normal place.
	if(LLSelectMgr::getInstance()->getSelection()->isEmpty())
	{
		return;
	}
	
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
	if ( (rlv_handler_t::isEnabled()) && (!rlvCanDeleteOrReturn()) )
	{
		return;
	}
// [/RLVa:KB]

	BOOL you_own_everything = TRUE;
	BOOL locked_but_takeable_object = FALSE;
	LLUUID category_id;
	
	for (LLObjectSelection::root_iterator iter = LLSelectMgr::getInstance()->getSelection()->root_begin();
		 iter != LLSelectMgr::getInstance()->getSelection()->root_end(); iter++)
	{
		LLSelectNode* node = *iter;
		LLViewerObject* object = node->getObject();
		if(object)
		{
			if(!object->permYouOwner())
			{
				you_own_everything = FALSE;
			}

			if(!object->permMove())
			{
				locked_but_takeable_object = TRUE;
			}
		}
		if(node->mFolderID.notNull())
		{
			if(category_id.isNull())
			{
				category_id = node->mFolderID;
			}
			else if(category_id != node->mFolderID)
			{
				// we have found two potential destinations. break out
				// now and send to the default location.
				category_id.setNull();
				break;
			}
		}
	}
	if(category_id.notNull())
	{
		// there is an unambiguous destination. See if this agent has
		// such a location and it is not in the trash or library
		if(!gInventory.getCategory(category_id))
		{
			// nope, set to NULL.
			category_id.setNull();
		}
		if(category_id.notNull())
		{
		        // check trash
			LLUUID trash;
			trash = gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH);
			if(category_id == trash || gInventory.isObjectDescendentOf(category_id, trash))
			{
				category_id.setNull();
			}

			// check library
			if(gInventory.isObjectDescendentOf(category_id, gInventoryLibraryRoot))
			{
				category_id.setNull();
			}

		}
	}
	if(category_id.isNull())
	{
		category_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_OBJECT);
	}
	LLUUID* cat_id = new LLUUID(category_id);
	if(locked_but_takeable_object ||
	   !you_own_everything)
	{
		if(locked_but_takeable_object && you_own_everything)
		{
			gViewerWindow->alertXml("ConfirmObjectTakeLock",
			confirm_take,
			(void*)cat_id);

		}
		else if(!locked_but_takeable_object && !you_own_everything)
		{
			gViewerWindow->alertXml("ConfirmObjectTakeNoOwn",
			confirm_take,
			(void*)cat_id);
		}
		else
		{
			gViewerWindow->alertXml("ConfirmObjectTakeLockNoOwn",
			confirm_take,
			(void*)cat_id);
		}


	}

	else
	{
		confirm_take(0, (void*)cat_id);
	}
}

void confirm_take(S32 option, void* data)
{
	LLUUID* cat_id = (LLUUID*)data;
	if(!cat_id) return;
	if(enable_take() && (option == 0))
	{
		derez_objects(DRD_TAKE_INTO_AGENT_INVENTORY, *cat_id);
	}
	delete cat_id;
}

// You can take an item when it is public and transferrable, or when
// you own it. We err on the side of enabling the item when at least
// one item selected can be copied to inventory.
BOOL enable_take()
{
	if (sitting_on_selection())
	{
		return FALSE;
	}

// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
	if ( (rlv_handler_t::isEnabled()) && (!rlvCanDeleteOrReturn()) )
	{
		return FALSE;
	}
// [/RLVa:KB]

	for (LLObjectSelection::valid_root_iterator iter = LLSelectMgr::getInstance()->getSelection()->valid_root_begin();
		 iter != LLSelectMgr::getInstance()->getSelection()->valid_root_end(); iter++)
	{
		LLSelectNode* node = *iter;
		LLViewerObject* object = node->getObject();
		if (object->isAvatar())
		{
			// ...don't acquire avatars
			continue;
		}

#ifdef HACKED_GODLIKE_VIEWER
		return TRUE;
#else
# ifdef TOGGLE_HACKED_GODLIKE_VIEWER
		if (!LLViewerLogin::getInstance()->isInProductionGrid() 
            && gAgent.isGodlike())
		{
			return TRUE;
		}
# endif
		if((node->mPermissions->allowTransferTo(gAgent.getID())
			&& object->permModify())
		   || (node->mPermissions->getOwner() == gAgent.getID()))
		{
			return TRUE;
		}
#endif
	}
	return FALSE;
}

class LLToolsTake : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (LLSelectMgr::getInstance()->getSelection()->isEmpty())
		{
			return true;
		}
		else
		{
			handle_take();
			return true;
		}
	}
};

class LLToolsEnableTake : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = enable_take();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

void callback_show_buy_currency(S32 option, void*)
{
	if (0 == option)
	{
		llinfos << "Loading page " << BUY_CURRENCY_URL << llendl;
		LLWeb::loadURL(BUY_CURRENCY_URL);
	}
}


void show_buy_currency(const char* extra)
{
	// Don't show currency web page for branded clients.

	std::ostringstream mesg;
	if (extra != NULL)
	{	
		mesg << extra << "\n \n";
	}
	mesg << "Go to " << BUY_CURRENCY_URL << "\nfor information on purchasing currency?";

	LLStringUtil::format_map_t args;
	if (extra != NULL)
	{
		args["[EXTRA]"] = extra;
	}
	args["[URL]"] = BUY_CURRENCY_URL;
	gViewerWindow->alertXml("PromptGoToCurrencyPage", args, 
		callback_show_buy_currency);
}

void handle_buy_currency(void*)
{
//	LLFloaterBuyCurrency::buyCurrency();
}

void handle_buy(void*)
{
	if (LLSelectMgr::getInstance()->getSelection()->isEmpty()) return;

	LLSaleInfo sale_info;
	BOOL valid = LLSelectMgr::getInstance()->selectGetSaleInfo(sale_info);
	if (!valid) return;

	if (sale_info.getSaleType() == LLSaleInfo::FS_CONTENTS)
	{
		handle_buy_contents(sale_info);
	}
	else
	{
		handle_buy_object(sale_info);
	}
}

class LLObjectBuy : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_buy(NULL);
		return true;
	}
};

BOOL sitting_on_selection()
{
	LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode();
	if (!node)
	{
		return FALSE;
	}

	if (!node->mValid)
	{
		return FALSE;
	}

	LLViewerObject* root_object = node->getObject();
	if (!root_object)
	{
		return FALSE;
	}

	// Need to determine if avatar is sitting on this object
	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if (!avatar)
	{
		return FALSE;
	}

	return (avatar->mIsSitting && avatar->getRoot() == root_object);
}

class LLToolsSaveToInventory : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if(enable_save_into_inventory(NULL))
		{
			derez_objects(DRD_SAVE_INTO_AGENT_INVENTORY, LLUUID::null);
		}
		return true;
	}
};

class LLToolsSaveToObjectInventory : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode();
		if(node && (node->mValid) && (!node->mFromTaskID.isNull()))
		{
			// *TODO: check to see if the fromtaskid object exists.
			derez_objects(DRD_SAVE_INTO_TASK_INVENTORY, node->mFromTaskID);
		}
		return true;
	}
};

// Round the position of all root objects to the grid
class LLToolsSnapObjectXY : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		F64 snap_size = (F64)gSavedSettings.getF32("GridResolution");

		for (LLObjectSelection::root_iterator iter = LLSelectMgr::getInstance()->getSelection()->root_begin();
			 iter != LLSelectMgr::getInstance()->getSelection()->root_end(); iter++)
		{
			LLSelectNode* node = *iter;
			LLViewerObject* obj = node->getObject();
			if (obj->permModify())
			{
				LLVector3d pos_global = obj->getPositionGlobal();
				F64 round_x = fmod(pos_global.mdV[VX], snap_size);
				if (round_x < snap_size * 0.5)
				{
					// closer to round down
					pos_global.mdV[VX] -= round_x;
				}
				else
				{
					// closer to round up
					pos_global.mdV[VX] -= round_x;
					pos_global.mdV[VX] += snap_size;
				}

				F64 round_y = fmod(pos_global.mdV[VY], snap_size);
				if (round_y < snap_size * 0.5)
				{
					pos_global.mdV[VY] -= round_y;
				}
				else
				{
					pos_global.mdV[VY] -= round_y;
					pos_global.mdV[VY] += snap_size;
				}

				obj->setPositionGlobal(pos_global, FALSE);
			}
		}
		LLSelectMgr::getInstance()->sendMultipleUpdate(UPD_POSITION);
		return true;
	}
};

// in order to link, all objects must have the same owner, and the
// agent must have the ability to modify all of the objects. However,
// we're not answering that question with this method. The question
// we're answering is: does the user have a reasonable expectation
// that a link operation should work? If so, return true, false
// otherwise. this allows the handle_link method to more finely check
// the selection and give an error message when the uer has a
// reasonable expectation for the link to work, but it will fail.
class LLToolsEnableLink : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = false;
		// check if there are at least 2 objects selected, and that the
		// user can modify at least one of the selected objects.

		// in component mode, can't link
		if (!gSavedSettings.getBOOL("EditLinkedParts"))
		{
			if(LLSelectMgr::getInstance()->selectGetAllRootsValid() && LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() >= 2)
			{
				struct f : public LLSelectedObjectFunctor
				{
					virtual bool apply(LLViewerObject* object)
					{
						return object->permModify();
					}
				} func;
				const bool firstonly = true;
				new_value = LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func, firstonly);
			}
		}
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLToolsLink : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if(!LLSelectMgr::getInstance()->selectGetAllRootsValid())
		{
			LLNotifyBox::showXml("UnableToLinkWhileDownloading");
			return true;
		}

		S32 object_count = LLSelectMgr::getInstance()->getSelection()->getObjectCount();
		if (object_count > MAX_CHILDREN_PER_TASK + 1)
		{
			LLStringUtil::format_map_t args;
			args["[COUNT]"] = llformat("%d", object_count);
			int max = MAX_CHILDREN_PER_TASK+1;
			args["[MAX]"] = llformat("%d", max);
			gViewerWindow->alertXml("UnableToLinkObjects", args);
			return true;
		}

		if(LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() < 2)
		{
			gViewerWindow->alertXml("CannotLinkIncompleteSet");
			return true;
		}
		if(!LLSelectMgr::getInstance()->selectGetRootsModify())
		{
			gViewerWindow->alertXml("CannotLinkModify");
			return true;
		}
		LLUUID owner_id;
		std::string owner_name;
		if(!LLSelectMgr::getInstance()->selectGetOwner(owner_id, owner_name))
		{
			// we don't actually care if you're the owner, but novices are
			// the most likely to be stumped by this one, so offer the
			// easiest and most likely solution.
			gViewerWindow->alertXml("CannotLinkDifferentOwners");
			return true;
		}
		LLSelectMgr::getInstance()->sendLink();
		return true;
	}
};

class LLToolsEnableUnlink : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = false;
		if (LLToolMgr::getInstance()->getCurrentTool() != LLToolFace::getInstance())
		{
			if (LLSelectMgr::getInstance()->selectGetAllRootsValid() &&
				LLSelectMgr::getInstance()->getSelection()->getFirstEditableObject() &&
				!LLSelectMgr::getInstance()->getSelection()->getFirstEditableObject()->isAttachment())
			{
				// LL's viewer unlinks the last linkset selected, 
				// regardless of how many linksets or prims are selected total. 
				// Preserve that behavior when enabling the unlink option.  
				if (gSavedSettings.getBOOL("EditLinkedParts"))
				{
					struct f : public LLSelectedNodeFunctor
					{
						virtual bool apply(LLSelectNode* pNode)
						{
							// Return the first selection node that is
							//    1) not a root prim
							//    2) or a root prim that has child prims
							// or in other words: any prim that is part of a linkset
							return (pNode->getObject() != pNode->getObject()->getRootEdit()) || 
									(pNode->getObject()->numChildren() != 0);
						}
					} func;

					if (LLSelectMgr::getInstance()->getSelection()->getFirstRootNode(&func, TRUE))
					{
						// the selection contains at least one prim (child or root) that is part of a linkset
						new_value = true;
					}
				}
				else
				{
					if (LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() != 
						LLSelectMgr::getInstance()->getSelection()->getObjectCount())
					{
						new_value = true;
					}
				}
			}
		}

// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g) | Modified: RLVa-0.2.0g
		// The user might not be allowed to unlink this object due to RLV settings,
		// because it would unsit them if they are sitting on the object.
		if ( (new_value) && (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) && 
			 (gAgent.getAvatarObject()) && (gAgent.getAvatarObject()->mIsSitting) )
		{
			// Allow if the avie isn't sitting on any of the selected objects
			LLObjectSelectionHandle handleSel = LLSelectMgr::getInstance()->getSelection();
			RlvSelectIsSittingOn func(gAgent.getAvatarObject()->getRoot());
			if (handleSel->getFirstRootNode(&func, TRUE))
				new_value = false;
		}
// [/RLVa:KB]

		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLToolsUnlink : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g) | Modified: RLVa-0.2.0g
		// The user might not be allowed to unlink this object due to RLV settings,
		// because it would unsit them if they are sitting on the object.
		if ( (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) && (gAgent.getAvatarObject()) && (gAgent.getAvatarObject()->mIsSitting) )
		{
			// Allow if the avie isn't sitting on any of the selected objects
			LLObjectSelectionHandle handleSel = LLSelectMgr::getInstance()->getSelection();
			RlvSelectIsSittingOn func(gAgent.getAvatarObject()->getRoot());
			if (handleSel->getFirstRootNode(&func, TRUE))
				return true;
		}
// [/RLVa:KB]

		LLSelectMgr::getInstance()->sendDelink();
		return true;
	}
};


class LLToolsStopAllAnimations : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gAgent.stopCurrentAnimations();
		return true;
	}
};

class LLToolsReleaseKeys : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
		if ( (rlv_handler_t::isEnabled()) && (gRlvHandler.hasLockedAttachment()) )
		{
			return true;
		}
// [/RLVa:KB]

		gAgent.forceReleaseControls();

		return true;
	}
};

class LLToolsEnableReleaseKeys : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
		gMenuHolder->findControl(userdata["control"].asString())->setValue( 
				gAgent.anyControlGrabbed() && ( (!rlv_handler_t::isEnabled()) || (!gRlvHandler.hasLockedAttachment() ) ) );
// [/RLVa:KB]
		//gMenuHolder->findControl(userdata["control"].asString())->setValue( gAgent.anyControlGrabbed() );
		return true;
	}
};

//void handle_hinge(void*)
//{
//	LLSelectMgr::getInstance()->sendHinge(1);
//}

//void handle_ptop(void*)
//{
//	LLSelectMgr::getInstance()->sendHinge(2);
//}

//void handle_lptop(void*)
//{
//	LLSelectMgr::getInstance()->sendHinge(3);
//}

//void handle_wheel(void*)
//{
//	LLSelectMgr::getInstance()->sendHinge(4);
//}

//void handle_dehinge(void*)
//{
//	LLSelectMgr::getInstance()->sendDehinge();
//}

//BOOL enable_dehinge(void*)
//{
//	LLViewerObject* obj = LLSelectMgr::getInstance()->getSelection()->getFirstEditableObject();
//	return obj && !obj->isAttachment();
//}


class LLEditEnableCut : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canCut();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditCut : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if( LLEditMenuHandler::gEditMenuHandler )
		{
			LLEditMenuHandler::gEditMenuHandler->cut();
		}
		return true;
	}
};

class LLEditEnableCopy : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canCopy();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditCopy : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if( LLEditMenuHandler::gEditMenuHandler )
		{
			LLEditMenuHandler::gEditMenuHandler->copy();
		}
		return true;
	}
};

class LLEditEnablePaste : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canPaste();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditPaste : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if( LLEditMenuHandler::gEditMenuHandler )
		{
			LLEditMenuHandler::gEditMenuHandler->paste();
		}
		return true;
	}
};

class LLEditEnableDelete : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canDoDelete();

// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		// NOTE: we want to disable delete on objects but not disable delete on text
		if ( (new_value) && (rlv_handler_t::isEnabled()) && (LLEditMenuHandler::gEditMenuHandler == LLSelectMgr::getInstance()) )
		{
			new_value = rlvCanDeleteOrReturn();
		}
// [/RLVa:KB]

		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditDelete : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		// NOTE: we want to disable delete on objects but not disable delete on text
		if ( (rlv_handler_t::isEnabled()) && (LLEditMenuHandler::gEditMenuHandler == LLSelectMgr::getInstance()) && 
			 (!rlvCanDeleteOrReturn()) )
		{
			return true;
		}
// [/RLVa:KB]

		// If a text field can do a deletion, it gets precedence over deleting
		// an object in the world.
		if( LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canDoDelete())
		{
			LLEditMenuHandler::gEditMenuHandler->doDelete();
		}

		// and close any pie/context menus when done
		gMenuHolder->hideMenus();

		// When deleting an object we may not actually be done
		// Keep selection so we know what to delete when confirmation is needed about the delete
		gPieObject->hide(TRUE);
		return true;
	}
};

class LLObjectEnableDelete : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = 
#ifdef HACKED_GODLIKE_VIEWER
			TRUE;
#else
# ifdef TOGGLE_HACKED_GODLIKE_VIEWER
			(!LLViewerLogin::getInstance()->isInProductionGrid()
             && gAgent.isGodlike()) ||
# endif
			LLSelectMgr::getInstance()->canDoDelete();
#endif
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		if ( (new_value) && (rlv_handler_t::isEnabled()) )
		{
			new_value = rlvCanDeleteOrReturn();
		}
// [/RLVa:KB]
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditSearch : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLFloaterDirectory::toggleFind(NULL);
		return true;
	}
};

class LLObjectDelete : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		if ( (rlv_handler_t::isEnabled()) && (!rlvCanDeleteOrReturn()) )
		{
			return true;
		}
// [/RLVa:KB]

		if (LLSelectMgr::getInstance())
		{
			LLSelectMgr::getInstance()->doDelete();
		}

		// and close any pie/context menus when done
		gMenuHolder->hideMenus();

		// When deleting an object we may not actually be done
		// Keep selection so we know what to delete when confirmation is needed about the delete
		gPieObject->hide(TRUE);
		return true;
	}
};

void handle_force_delete(void*)
{
	LLSelectMgr::getInstance()->selectForceDelete();
}

class LLViewEnableLastChatter : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// *TODO: add check that last chatter is in range
		bool new_value = (gAgent.cameraThirdPerson() && gAgent.getLastChatter().notNull());
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditEnableDeselect : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canDeselect();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditDeselect : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if( LLEditMenuHandler::gEditMenuHandler )
		{
			LLEditMenuHandler::gEditMenuHandler->deselect();
		}
		return true;
	}
};

class LLEditEnableSelectAll : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canSelectAll();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};


class LLEditSelectAll : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if( LLEditMenuHandler::gEditMenuHandler )
		{
			LLEditMenuHandler::gEditMenuHandler->selectAll();
		}
		return true;
	}
};


class LLEditEnableUndo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canUndo();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditUndo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if( LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canUndo() )
		{
			LLEditMenuHandler::gEditMenuHandler->undo();
		}
		return true;
	}
};

class LLEditEnableRedo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canRedo();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditRedo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if( LLEditMenuHandler::gEditMenuHandler && LLEditMenuHandler::gEditMenuHandler->canRedo() )
		{
			LLEditMenuHandler::gEditMenuHandler->redo();
		}
		return true;
	}
};



void print_object_info(void*)
{
	LLSelectMgr::getInstance()->selectionDump();
}

void print_agent_nvpairs(void*)
{
	LLViewerObject *objectp;

	llinfos << "Agent Name Value Pairs" << llendl;

	objectp = gObjectList.findObject(gAgentID);
	if (objectp)
	{
		objectp->printNameValuePairs();
	}
	else
	{
		llinfos << "Can't find agent object" << llendl;
	}

	llinfos << "Camera at " << gAgent.getCameraPositionGlobal() << llendl;
}

class LLViewToggleAdvanced : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		toggle_debug_menus(NULL);
		return true;
	}
};

class LLViewCheckAdvanced : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		BOOL new_value = gSavedSettings.getBOOL("UseDebugMenus");
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

void show_debug_menus()
{
	// this can get called at login screen where there is no menu so only toggle it if one exists
	if ( gMenuBarView )
	{
		BOOL debug = gSavedSettings.getBOOL("UseDebugMenus");
		
		if(debug)
		{
			LLFirstUse::useDebugMenus();
		}

		gMenuBarView->setItemVisible(CLIENT_MENU_NAME, debug);
		gMenuBarView->setItemEnabled(CLIENT_MENU_NAME, debug);

		// Server ('Admin') menu hidden when not in godmode.
		const bool show_server_menu = debug && (gAgent.getGodLevel() > GOD_NOT);
		gMenuBarView->setItemVisible(SERVER_MENU_NAME, show_server_menu);
		gMenuBarView->setItemEnabled(SERVER_MENU_NAME, show_server_menu);

		//gMenuBarView->setItemVisible("DebugOptions",	visible);
		//gMenuBarView->setItemVisible(std::string(AVI_TOOLS),	visible);

		gMenuBarView->arrange(); // clean-up positioning 
	};
}

void toggle_debug_menus(void*)
{
	BOOL visible = ! gSavedSettings.getBOOL("UseDebugMenus");
	gSavedSettings.setBOOL("UseDebugMenus", visible);
	show_debug_menus();
}


// LLUUID gExporterRequestID;
// std::string gExportDirectory;

// LLUploadDialog *gExportDialog = NULL;

// void handle_export_selected( void * )
// {
// 	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
// 	if (selection->isEmpty())
// 	{
// 		return;
// 	}
// 	llinfos << "Exporting selected objects:" << llendl;

// 	gExporterRequestID.generate();
// 	gExportDirectory = "";

// 	LLMessageSystem* msg = gMessageSystem;
// 	msg->newMessageFast(_PREHASH_ObjectExportSelected);
// 	msg->nextBlockFast(_PREHASH_AgentData);
// 	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
// 	msg->addUUIDFast(_PREHASH_RequestID, gExporterRequestID);
// 	msg->addS16Fast(_PREHASH_VolumeDetail, 4);

// 	for (LLObjectSelection::root_iterator iter = selection->root_begin();
// 		 iter != selection->root_end(); iter++)
// 	{
// 		LLSelectNode* node = *iter;
// 		LLViewerObject* object = node->getObject();
// 		msg->nextBlockFast(_PREHASH_ObjectData);
// 		msg->addUUIDFast(_PREHASH_ObjectID, object->getID());
// 		llinfos << "Object: " << object->getID() << llendl;
// 	}
// 	msg->sendReliable(gAgent.getRegion()->getHost());

// 	gExportDialog = LLUploadDialog::modalUploadDialog("Exporting selected objects...");
// }

BOOL menu_check_build_tool( void* user_data )
{
	S32 index = (intptr_t) user_data;
	return LLToolMgr::getInstance()->getCurrentToolset()->isToolSelected( index );
}

void handle_reload_settings(void*)
{
	gSavedSettings.resetToDefaults();
	gSavedSettings.loadFromFile(gSavedSettings.getString("ClientSettingsFile"));

	llinfos << "Loading colors from colors.xml" << llendl;
	std::string color_file = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,"colors.xml");
	gColors.resetToDefaults();
	gColors.loadFromFileLegacy(color_file, FALSE, TYPE_COL4U);
}

class LLWorldSetHomeLocation : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// we just send the message and let the server check for failure cases
		// server will echo back a "Home position set." alert if it succeeds
		// and the home location screencapture happens when that alert is recieved
		gAgent.setStartPosition(START_LOCATION_ID_HOME);
		return true;
	}
};

class LLWorldTeleportHome : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gAgent.teleportHomeConfirm();
		return true;
	}
};

class LLWorldAlwaysRun : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// as well as altering the default walk-vs-run state,
		// we also change the *current* walk-vs-run state.
		if (gAgent.getAlwaysRun())
		{
			gAgent.clearAlwaysRun();
			gAgent.clearRunning();
		}
		else
		{
			gAgent.setAlwaysRun();
			gAgent.setRunning();
		}

		// tell the simulator.
		gAgent.sendWalkRun(gAgent.getAlwaysRun());

		return true;
	}
};

class LLWorldCheckAlwaysRun : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gAgent.getAlwaysRun();
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLWorldSetAway : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (gAgent.getAFK())
		{
			gAgent.clearAFK();
		}
		else
		{
			gAgent.setAFK();
		}
		return true;
	}
};

class LLWorldSetBusy : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (gAgent.getBusy())
		{
			gAgent.clearBusy();
		}
		else
		{
			gAgent.setBusy();
			gViewerWindow->alertXml("BusyModeSet");
		}
		return true;
	}
};


class LLWorldCreateLandmark : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-04 (RLVa-1.0.0a)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWLOC))
		{
			return true;
		}
// [/RLVa:KB]
		LLViewerRegion* agent_region = gAgent.getRegion();
		if(!agent_region)
		{
			llwarns << "No agent region" << llendl;
			return true;
		}
		LLParcel* agent_parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
		if (!agent_parcel)
		{
			llwarns << "No agent parcel" << llendl;
			return true;
		}
		if (!agent_parcel->getAllowLandmark()
			&& !LLViewerParcelMgr::isParcelOwnedByAgent(agent_parcel, GP_LAND_ALLOW_LANDMARK))
		{
			gViewerWindow->alertXml("CannotCreateLandmarkNotOwner");
			return true;
		}

		LLChat chat;

		LLUUID folder_id;
		folder_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_LANDMARK);
		std::string pos_string;
		gAgent.buildLocationString(pos_string);

		std::string log_message = LLTrans::getString("landmark_created") + " ";
		log_message += pos_string;
		chat.mText = log_message;
		LLFloaterChat::addChat(chat, FALSE, FALSE);
		
		create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
							  folder_id, LLTransactionID::tnull,
							  pos_string, pos_string, // name, desc
							  LLAssetType::AT_LANDMARK,
							  LLInventoryType::IT_LANDMARK,
							  NOT_WEARABLE, PERM_ALL, 
							  new LLCreateLandmarkCallback);
		return true;
	}
};

class LLToolsLookAtSelection : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		const F32 PADDING_FACTOR = 2.f;
		BOOL zoom = (userdata.asString() == "zoom");
		if (!LLSelectMgr::getInstance()->getSelection()->isEmpty())
		{
			gAgent.setFocusOnAvatar(FALSE, ANIMATE);

			LLBBox selection_bbox = LLSelectMgr::getInstance()->getBBoxOfSelection();
			F32 angle_of_view = llmax(0.1f, LLViewerCamera::getInstance()->getAspect() > 1.f ? LLViewerCamera::getInstance()->getView() * LLViewerCamera::getInstance()->getAspect() : LLViewerCamera::getInstance()->getView());
			F32 distance = selection_bbox.getExtentLocal().magVec() * PADDING_FACTOR / atan(angle_of_view);

			LLVector3 obj_to_cam = LLViewerCamera::getInstance()->getOrigin() - selection_bbox.getCenterAgent();
			obj_to_cam.normVec();

			LLUUID object_id;
			if (LLSelectMgr::getInstance()->getSelection()->getPrimaryObject())
			{
				object_id = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject()->mID;
			}
			if (zoom)
			{
				gAgent.setCameraPosAndFocusGlobal(LLSelectMgr::getInstance()->getSelectionCenterGlobal() + LLVector3d(obj_to_cam * distance), 
												LLSelectMgr::getInstance()->getSelectionCenterGlobal(), 
												object_id );
			}
			else
			{
				gAgent.setFocusGlobal( LLSelectMgr::getInstance()->getSelectionCenterGlobal(), object_id );
			}
		}
		return true;
	}
};

void callback_invite_to_group(LLUUID group_id, void *user_data)
{
	std::vector<LLUUID> agent_ids;
	agent_ids.push_back(*(LLUUID *)user_data);
	
	LLFloaterGroupInvite::showForGroup(group_id, &agent_ids);
}

void invite_to_group(const LLUUID& dest_id)
{
	LLViewerObject* dest = gObjectList.findObject(dest_id);
	if(dest && dest->isAvatar())
	{
		LLFloaterGroupPicker* widget;
		widget = LLFloaterGroupPicker::showInstance(LLSD(gAgent.getID()));
		if (widget)
		{
			widget->center();
			widget->setPowersMask(GP_MEMBER_INVITE);
			widget->setSelectCallback(callback_invite_to_group, (void *)&dest_id);
		}
	}
}

class LLAvatarInviteToGroup : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
		{
			return true;
		}
// [/RLVa:KB]

		LLVOAvatar* avatar = find_avatar_from_object( LLSelectMgr::getInstance()->getSelection()->getPrimaryObject() );
		if(avatar)
		{
			invite_to_group(avatar->getID());
		}
		return true;
	}
};

class LLAvatarAddFriend : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
		{
			return true;	// Fallback code [see LLAvatarEnableAddFriend::handleEvent()]
		}
// [/RLVa:KB]

		LLVOAvatar* avatar = find_avatar_from_object( LLSelectMgr::getInstance()->getSelection()->getPrimaryObject() );
		if(avatar && !is_agent_friend(avatar->getID()))
		{
			request_friendship(avatar->getID());
		}
		return true;
	}
};

void complete_give_money(S32 option, void* user_data)
{
	if (option == 0)
	{
		gAgent.clearBusy();
	}

	LLObjectSelectionHandle handle(*(LLObjectSelectionHandle*)user_data);
	delete (LLObjectSelectionHandle*)user_data;

	LLViewerObject* objectp = handle->getPrimaryObject();

	// Show avatar's name if paying attachment
	if (objectp && objectp->isAttachment())
	{
		while (objectp && !objectp->isAvatar())
		{
			objectp = (LLViewerObject*)objectp->getParent();
		}
	}

	if (objectp)
	{
		if (objectp->isAvatar())
		{
			const BOOL is_group = FALSE;
			LLFloaterPay::payDirectly(&give_money,
									  objectp->getID(),
									  is_group);
		}
		else
		{
			LLFloaterPay::payViaObject(&give_money, objectp->getID());
		}
	}
}

bool handle_give_money_dialog()
{
	LLObjectSelectionHandle* handlep = new LLObjectSelectionHandle(LLSelectMgr::getInstance()->getSelection());
	if (gAgent.getBusy())
	{
		// warn users of being in busy mode during a transaction
		gViewerWindow->alertXml("BusyModePay", complete_give_money, handlep);
	}
	else
	{
		complete_give_money(1, handlep);
	}
	return true;
}

class LLPayObject : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		return handle_give_money_dialog();
	}
};

class LLEnablePayObject : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar* avatar = find_avatar_from_object(LLSelectMgr::getInstance()->getSelection()->getPrimaryObject());
		bool new_value = (avatar != NULL);
		if (!new_value)
		{
			LLViewerObject* object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
			if( object )
			{
				LLViewerObject *parent = (LLViewerObject *)object->getParent();
				if((object->flagTakesMoney()) || (parent && parent->flagTakesMoney()))
				{
					new_value = true;
				}
			}
		}

// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
		// Don't enable "Pay..." on the avatar pie menu under @shownames=n 
		new_value &= (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES)) || (avatar == NULL);
// [/RLVa:KB]

		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLObjectEnableSitOrStand : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = false;
		LLViewerObject* dest_object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();

		if(dest_object)
		{
			if(dest_object->getPCode() == LL_PCODE_VOLUME)
			{
				new_value = true;
			}
		}
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);

		// Update label
		std::string label;
		std::string sit_text;
		std::string stand_text;
		std::string param = userdata["data"].asString();
		std::string::size_type offset = param.find(",");
		if (offset != param.npos)
		{
			sit_text = param.substr(0, offset);
			stand_text = param.substr(offset+1);
		}
		if (sitting_on_selection())
		{
			label = stand_text;
		}
		else
		{
			LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode();
			if (node && node->mValid && !node->mSitName.empty())
			{
				label.assign(node->mSitName);
			}
			else
			{
				label = sit_text;
			}
		}
		gMenuHolder->childSetText("Object Sit", label);

		return true;
	}
};

void edit_ui(void*)
{
	LLFloater::setEditModeEnabled(!LLFloater::getEditModeEnabled());
}

void dump_select_mgr(void*)
{
	LLSelectMgr::getInstance()->dump();
}

void dump_inventory(void*)
{
	gInventory.dumpInventory();
}

// forcibly unlock an object
void handle_force_unlock(void*)
{
	// First, make it public.
	LLSelectMgr::getInstance()->sendOwner(LLUUID::null, LLUUID::null, TRUE);

	// Second, lie to the viewer and mark it editable and unowned

	struct f : public LLSelectedObjectFunctor
	{
		virtual bool apply(LLViewerObject* object)
		{
			object->mFlags |= FLAGS_OBJECT_MOVE;
			object->mFlags |= FLAGS_OBJECT_MODIFY;
			object->mFlags |= FLAGS_OBJECT_COPY;

			object->mFlags &= ~FLAGS_OBJECT_ANY_OWNER;
			object->mFlags &= ~FLAGS_OBJECT_YOU_OWNER;
			return true;
		}
	} func;
	LLSelectMgr::getInstance()->getSelection()->applyToObjects(&func);
}

void handle_dump_followcam(void*)
{
	LLFollowCamMgr::dump();
}

void handle_viewer_enable_message_log(void*)
{
	gMessageSystem->startLogging();
}

void handle_viewer_disable_message_log(void*)
{
	gMessageSystem->stopLogging();
}

// TomY TODO: Move!
class LLShowFloater : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string floater_name = userdata.asString();
		if (floater_name == "gestures")
		{
			LLFloaterGesture::toggleVisibility();
		}
		else if (floater_name == "appearance")
		{
			if (gAgent.getWearablesLoaded())
			{
				gAgent.changeCameraToCustomizeAvatar();
			}
		}
		else if (floater_name == "friends")
		{
			LLFloaterMyFriends::toggleInstance(0);
		}
		else if (floater_name == "preferences")
		{
			LLFloaterPreference::show(NULL);
		}
		else if (floater_name == "toolbar")
		{
			LLToolBar::toggle(NULL);
		}
		else if (floater_name == "chat history")
		{
			LLFloaterChat::toggleInstance(LLSD());
		}
		else if (floater_name == "im")
		{
			LLFloaterChatterBox::toggleInstance(LLSD());
		}
		else if (floater_name == "inventory")
		{
			LLInventoryView::toggleVisibility(NULL);
		}
		else if (floater_name == "mute list")
		{
			LLFloaterMute::toggleInstance();
		}
		else if (floater_name == "camera controls")
		{
			LLFloaterCamera::toggleInstance();
		}
		else if (floater_name == "movement controls")
		{
			LLFloaterMove::toggleInstance();
		}
		else if (floater_name == "world map")
		{
			LLFloaterWorldMap::toggle(NULL);
		}
		else if (floater_name == "mini map")
		{
			LLFloaterMap::toggleInstance();
		}
		else if (floater_name == "stat bar")
		{
			gDebugView->mFloaterStatsp->setVisible(!gDebugView->mFloaterStatsp->getVisible());
		}
		else if (floater_name == "my land")
		{
			LLFloaterLandHoldings::show(NULL);
		}
		else if (floater_name == "about land")
		{
			if (LLViewerParcelMgr::getInstance()->selectionEmpty())
			{
				LLViewerParcelMgr::getInstance()->selectParcelAt(gAgent.getPositionGlobal());
			}

			LLFloaterLand::showInstance();
		}
		else if (floater_name == "buy land")
		{
// [RLVa:KB] - Checked: 2009-07-04 (RLVa-1.0.0a)
			if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWLOC))
			{
				return true;
			}
// [/RLVa:KB]
			if (LLViewerParcelMgr::getInstance()->selectionEmpty())
			{
				LLViewerParcelMgr::getInstance()->selectParcelAt(gAgent.getPositionGlobal());
			}
			
			LLViewerParcelMgr::getInstance()->startBuyLand();
		}
		else if (floater_name == "about region")
		{
			LLFloaterRegionInfo::showInstance();
		}
		else if (floater_name == "grid options")
		{
			LLFloaterBuildOptions::show(NULL);
		}
		else if (floater_name == "script errors")
		{
			LLFloaterScriptDebug::show(LLUUID::null);
		}
		else if (floater_name == "help f1")
		{
			LLFloaterMediaBrowser::helpF1();
		}
		else if (floater_name == "help tutorial")
		{
			LLFloaterHUD::showHUD();
		}
		else if (floater_name == "complaint reporter")
		{
			// Prevent menu from appearing in screen shot.
			gMenuHolder->hideMenus();
			LLFloaterReporter::showFromMenu(COMPLAINT_REPORT);
		}
		else if (floater_name == "mean events")
		{
			if (!gNoRender)
			{
				LLFloaterBump::show(NULL);
			}
		}
		else if (floater_name == "lag meter")
		{
			LLFloaterLagMeter::showInstance();
		}
		else if (floater_name == "bug reporter")
		{
			// Prevent menu from appearing in screen shot.
			gMenuHolder->hideMenus();
			LLFloaterReporter::showFromMenu(BUG_REPORT);
		}
		else if (floater_name == "buy currency")
		{
			LLFloaterBuyCurrency::buyCurrency();
		}
		else if (floater_name == "about")
		{
			LLFloaterAbout::show(NULL);
		}
		else if (floater_name == "active speakers")
		{
			LLFloaterActiveSpeakers::toggleInstance(LLSD());
		}
		else if (floater_name == "animation list")
		{
			JCFloaterAnimList::toggleInstance(LLSD());
		}
		else if (floater_name == "inworld browser")
		{
			LLFloaterMediaBrowser::toggle();
		}
		else if (floater_name == "beacons")
		{
			LLFloaterBeacons::toggleInstance(LLSD());
		}
		return true;
	}
};

class LLFloaterVisible : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string control_name = userdata["control"].asString();
		std::string floater_name = userdata["data"].asString();
		bool new_value = false;
		if (floater_name == "friends")
		{
			new_value = LLFloaterMyFriends::instanceVisible(0);
		}
		else if (floater_name == "communicate")
		{
			new_value = LLFloaterChatterBox::instanceVisible();
		}
		else if (floater_name == "toolbar")
		{
			new_value = LLToolBar::visible(NULL);
		}
		else if (floater_name == "chat history")
		{
			new_value = LLFloaterChat::instanceVisible();
		}
		else if (floater_name == "im")
		{
			new_value = LLFloaterMyFriends::instanceVisible(0);
		}
		else if (floater_name == "mute list")
		{
			new_value = LLFloaterMute::instanceVisible();
		}
		else if (floater_name == "camera controls")
		{
			new_value = LLFloaterCamera::instanceVisible();
		}
		else if (floater_name == "movement controls")
		{
			new_value = LLFloaterMove::instanceVisible();
		}
		else if (floater_name == "stat bar")
		{
			new_value = gDebugView->mFloaterStatsp->getVisible();
		}
		else if (floater_name == "active speakers")
		{
			new_value = LLFloaterActiveSpeakers::instanceVisible(LLSD());
		}
		else if (floater_name == "beacons")
		{
			new_value = LLFloaterBeacons::instanceVisible(LLSD());
		}
		else if (floater_name == "inventory")
		{
			LLInventoryView* iv = LLInventoryView::getActiveInventory(); 
			new_value = (NULL != iv && TRUE == iv->getVisible());
		}
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};

void callback_show_url(S32 option, void* data)
{
	std::string* urlp = (std::string*)data;
	if (0 == option)
	{
		LLWeb::loadURL(*urlp);
	}
	delete urlp;
}

class LLPromptShowURL : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string param = userdata.asString();
		std::string::size_type offset = param.find(",");
		if (offset != param.npos)
		{
			std::string alert = param.substr(0, offset);
			std::string url = param.substr(offset+1);
			std::string* url_copy = new std::string(url);
			gViewerWindow->alertXml(alert, callback_show_url, url_copy);
		}
		else
		{
			llinfos << "PromptShowURL invalid parameters! Expecting \"ALERT,URL\"." << llendl;
		}
		return true;
	}
};

class LLShowAgentProfile : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLUUID agent_id;
		if (userdata.asString() == "agent")
		{
			agent_id = gAgent.getID();
		}
		else if (userdata.asString() == "hit object")
		{
// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
			if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
			{
				return true;
			}
// [/RLVa:KB]

			LLViewerObject* objectp = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
			if (objectp)
			{
				agent_id = objectp->getID();
			}
		}
		else
		{
			agent_id = userdata.asUUID();
		}

		LLVOAvatar* avatar = find_avatar_from_object(agent_id);
		if (avatar)
		{
			LLFloaterAvatarInfo::show( avatar->getID() );
		}
		return true;
	}
};

class LLShowAgentGroups : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLFloaterMyFriends::toggleInstance(1);
		return true;
	}
};

void handle_focus(void *)
{
	if (gDisconnected)
	{
		return;
	}

	if (gAgent.getFocusOnAvatar())
	{
		// zoom in if we're looking at the avatar
		gAgent.setFocusOnAvatar(FALSE, ANIMATE);
		gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());
		gAgent.cameraZoomIn(0.666f);
	}
	else
	{
		gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());
	}

	gViewerWindow->moveCursorToCenter();

	// Switch to camera toolset
//	LLToolMgr::getInstance()->setCurrentToolset(gCameraToolset);
	LLToolMgr::getInstance()->getCurrentToolset()->selectTool( LLToolCamera::getInstance() );
}

class LLLandEdit : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-04 (RLVa-1.0.0b)
		if ( (rlv_handler_t::isEnabled()) && (gRlvHandler.hasBehaviour(RLV_BHVR_EDIT)) )
		{
			return true;
		}
// [/RLVa:KB]

		if (gAgent.getFocusOnAvatar() && gSavedSettings.getBOOL("EditCameraMovement") )
		{
			// zoom in if we're looking at the avatar
			gAgent.setFocusOnAvatar(FALSE, ANIMATE);
			gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());

			gAgent.cameraOrbitOver( F_PI * 0.25f );
			gViewerWindow->moveCursorToCenter();
		}
		else if ( gSavedSettings.getBOOL("EditCameraMovement") )
		{
			gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());
			gViewerWindow->moveCursorToCenter();
		}


		LLViewerParcelMgr::getInstance()->selectParcelAt( LLToolPie::getInstance()->getPick().mPosGlobal );

		gFloaterView->bringToFront( gFloaterTools );

		// Switch to land edit toolset
		LLToolMgr::getInstance()->getCurrentToolset()->selectTool( LLToolSelectLand::getInstance() );
		return true;
	}
};

class LLWorldEnableBuyLand : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLViewerParcelMgr::getInstance()->canAgentBuyParcel(
								LLViewerParcelMgr::getInstance()->selectionEmpty()
									? LLViewerParcelMgr::getInstance()->getAgentParcel()
									: LLViewerParcelMgr::getInstance()->getParcelSelection()->getParcel(),
								false);
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

BOOL enable_buy_land(void*)
{
	return LLViewerParcelMgr::getInstance()->canAgentBuyParcel(
				LLViewerParcelMgr::getInstance()->getParcelSelection()->getParcel(), false);
}


void handle_move(void*)
{
	if (gAgent.getFocusOnAvatar())
	{
		// zoom in if we're looking at the avatar
		gAgent.setFocusOnAvatar(FALSE, ANIMATE);
		gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());

		gAgent.cameraZoomIn(0.666f);
	}
	else
	{
		gAgent.setFocusGlobal(LLToolPie::getInstance()->getPick());
	}

	gViewerWindow->moveCursorToCenter();

	LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
	LLToolMgr::getInstance()->getCurrentToolset()->selectTool( LLToolGrab::getInstance() );
}

class LLObjectAttachToAvatar : public view_listener_t
{
public:
	static void setObjectSelection(LLObjectSelectionHandle selection) { sObjectSelection = selection; }

private:
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		setObjectSelection(LLSelectMgr::getInstance()->getSelection());
		LLViewerObject* selectedObject = sObjectSelection->getFirstRootObject();
		if (selectedObject)
		{
			S32 index = userdata.asInteger();
			LLViewerJointAttachment* attachment_point = NULL;
			if (index > 0)
				attachment_point = get_if_there(gAgent.getAvatarObject()->mAttachmentPoints, index, (LLViewerJointAttachment*)NULL);

// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
			if ( (rlv_handler_t::isEnabled()) &&
				 ( ((index == 0) && (gRlvHandler.hasLockedAttachment())) ||			 // Can't wear on default attach point
				   ((index > 0) && (!gRlvHandler.isDetachable(attachment_point))) || // Can't replace locked attachment
				   (gRlvHandler.hasBehaviour(RLV_BHVR_REZ)) ) )						 // Attach on rezzed object == "Take"
			{
				setObjectSelection(NULL); // Clear the selection or it'll get stuck
				return true;
			}
// [/RLVa:KB]

			confirm_replace_attachment(0, attachment_point);
		}
		return true;
	}

protected:
	static LLObjectSelectionHandle sObjectSelection;
};

LLObjectSelectionHandle LLObjectAttachToAvatar::sObjectSelection;

void near_attach_object(BOOL success, void *user_data)
{
	if (success)
	{
		LLViewerJointAttachment *attachment = (LLViewerJointAttachment *)user_data;
		
		U8 attachment_id = 0;
		if (attachment)
		{
			for (LLVOAvatar::attachment_map_t::iterator iter = gAgent.getAvatarObject()->mAttachmentPoints.begin();
				 iter != gAgent.getAvatarObject()->mAttachmentPoints.end(); ++iter)
			{
				if (iter->second == attachment)
				{
					attachment_id = iter->first;
					break;
				}
			}
		}
		else
		{
			// interpret 0 as "default location"
			attachment_id = 0;
		}
		LLSelectMgr::getInstance()->sendAttach(attachment_id);
	}		
	LLObjectAttachToAvatar::setObjectSelection(NULL);
}

void confirm_replace_attachment(S32 option, void* user_data)
{
	if (option == 0/*YES*/)
	{
		LLViewerObject* selectedObject = LLSelectMgr::getInstance()->getSelection()->getFirstRootObject();
		if (selectedObject)
		{
			const F32 MIN_STOP_DISTANCE = 1.f;	// meters
			const F32 ARM_LENGTH = 0.5f;		// meters
			const F32 SCALE_FUDGE = 1.5f;

			F32 stop_distance = SCALE_FUDGE * selectedObject->getMaxScale() + ARM_LENGTH;
			if (stop_distance < MIN_STOP_DISTANCE)
			{
				stop_distance = MIN_STOP_DISTANCE;
			}

			LLVector3 walkToSpot = selectedObject->getPositionAgent();
			
			// make sure we stop in front of the object
			LLVector3 delta = walkToSpot - gAgent.getPositionAgent();
			delta.normVec();
			delta = delta * 0.5f;
			walkToSpot -= delta;

			gAgent.startAutoPilotGlobal(gAgent.getPosGlobalFromAgent(walkToSpot), "Attach", NULL, near_attach_object, user_data, stop_distance);
			gAgent.clearFocusObject();
		}
	}
}

class LLAttachmentDrop : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// Called when the user clicked on an object attached to them
		// and selected "Drop".
		LLViewerObject *object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		if (!object)
		{
			llwarns << "handle_drop_attachment() - no object to drop" << llendl;
			return true;
		}

		LLViewerObject *parent = (LLViewerObject*)object->getParent();
		while (parent)
		{
			if(parent->isAvatar())
			{
				break;
			}
			object = parent;
			parent = (LLViewerObject*)parent->getParent();
		}

		if (!object)
		{
			llwarns << "handle_detach() - no object to detach" << llendl;
			return true;
		}

		if (object->isAvatar())
		{
			llwarns << "Trying to detach avatar from avatar." << llendl;
			return true;
		}

// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
		if (rlv_handler_t::isEnabled()) 
		{
			if (gRlvHandler.hasLockedAttachment())
			{
				// NOTE: copy/paste of the code in enable_detach()
				LLObjectSelectionHandle hSelect = LLSelectMgr::getInstance()->getSelection();
				RlvSelectHasLockedAttach functor;
				if ( (hSelect->isAttachment()) && (hSelect->getFirstRootNode(&functor, FALSE)) )
					return true;
			}
			else if (gRlvHandler.hasBehaviour(RLV_BHVR_REZ))
			{
				return true;
			}
		}
// [/RLVa:KB]

		// The sendDropAttachment() method works on the list of selected
		// objects.  Thus we need to clear the list, make sure it only
		// contains the object the user clicked, send the message,
		// then clear the list.
		LLSelectMgr::getInstance()->sendDropAttachment();
		return true;
	}
};

// called from avatar pie menu
void handle_detach_from_avatar(void* user_data)
{
	LLViewerJointAttachment *attachment = (LLViewerJointAttachment *)user_data;
	
	LLViewerObject* attached_object = attachment->getObject();

	if (attached_object)
	{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c) | Modified: RLVa-0.2.0d
		if ( (rlv_handler_t::isEnabled()) && (!gRlvHandler.isDetachable(attached_object)) )
		{
			return;
		}
// [/RLVa:KB]

		gMessageSystem->newMessage("ObjectDetach");
		gMessageSystem->nextBlockFast(_PREHASH_AgentData);
		gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());

		gMessageSystem->nextBlockFast(_PREHASH_ObjectData);
		gMessageSystem->addU32Fast(_PREHASH_ObjectLocalID, attached_object->getLocalID());
		gMessageSystem->sendReliable( gAgent.getRegionHost() );
	}
}

void attach_label(std::string& label, void* user_data)
{
	LLViewerJointAttachment* attachmentp = (LLViewerJointAttachment*)user_data;
	if (attachmentp)
	{
		label = attachmentp->getName();
		if (attachmentp->getObject())
		{
			LLViewerInventoryItem* itemp = gInventory.getItem(attachmentp->getItemID());
			if (itemp)
			{
				label += std::string(" (") + itemp->getName() + std::string(")");
			}
		}
	}
}

void detach_label(std::string& label, void* user_data)
{
	LLViewerJointAttachment* attachmentp = (LLViewerJointAttachment*)user_data;
	if (attachmentp)
	{
		label = attachmentp->getName();
		if (attachmentp->getObject())
		{
			LLViewerInventoryItem* itemp = gInventory.getItem(attachmentp->getItemID());
			if (itemp)
			{
				label += std::string(" (") + itemp->getName() + std::string(")");
			}
		}
	}
}


class LLAttachmentDetach : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// Called when the user clicked on an object attached to them
		// and selected "Detach".
		LLViewerObject *object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		if (!object)
		{
			llwarns << "handle_detach() - no object to detach" << llendl;
			return true;
		}

		LLViewerObject *parent = (LLViewerObject*)object->getParent();
		while (parent)
		{
			if(parent->isAvatar())
			{
				break;
			}
			object = parent;
			parent = (LLViewerObject*)parent->getParent();
		}

		if (!object)
		{
			llwarns << "handle_detach() - no object to detach" << llendl;
			return true;
		}

		if (object->isAvatar())
		{
			llwarns << "Trying to detach avatar from avatar." << llendl;
			return true;
		}

// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
		// NOTE: copy/paste of the code in enable_detach()
		if ( (rlv_handler_t::isEnabled()) && (gRlvHandler.hasLockedAttachment()) )
		{
			LLObjectSelectionHandle hSelect = LLSelectMgr::getInstance()->getSelection();
			RlvSelectHasLockedAttach functor;
			if ( (hSelect->isAttachment()) && (hSelect->getFirstRootNode(&functor, FALSE)) )
				return FALSE;
		}
// [/RLVa:KB]

		// The sendDetach() method works on the list of selected
		// objects.  Thus we need to clear the list, make sure it only
		// contains the object the user clicked, send the message,
		// then clear the list.
		// We use deselectAll to update the simulator's notion of what's
		// selected, and removeAll just to change things locally.
		//RN: I thought it was more useful to detach everything that was selected
		if (LLSelectMgr::getInstance()->getSelection()->isAttachment())
		{
			LLSelectMgr::getInstance()->sendDetach();
		}
		return true;
	}
};

//Adding an observer for a Jira 2422 and needs to be a fetch observer
//for Jira 3119
class LLWornItemFetchedObserver : public LLInventoryFetchObserver
{
public:
	LLWornItemFetchedObserver() {}
	virtual ~LLWornItemFetchedObserver() {}

protected:
	virtual void done()
	{
		gPieAttachment->buildDrawLabels();
		gInventory.removeObserver(this);
		delete this;
	}
};

// You can only drop items on parcels where you can build.
class LLAttachmentEnableDrop : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (gDisconnected)
			return true;
		LLParcel* parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
		BOOL can_build   = gAgent.isGodlike() || (parcel && parcel->getAllowModify());

		//Add an inventory observer to only allow dropping the newly attached item
		//once it exists in your inventory.  Look at Jira 2422.
		//-jwolk

		// A bug occurs when you wear/drop an item before it actively is added to your inventory
		// if this is the case (you're on a slow sim, etc.) a copy of the object,
		// well, a newly created object with the same properties, is placed
		// in your inventory.  Therefore, we disable the drop option until the
		// item is in your inventory

		LLViewerObject*              object         = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
		LLViewerJointAttachment*     attachment_pt  = NULL;
		LLInventoryItem*             item           = NULL;

		if ( object )
		{
    		S32 attachmentID  = ATTACHMENT_ID_FROM_STATE(object->getState());
			attachment_pt = get_if_there(gAgent.getAvatarObject()->mAttachmentPoints, attachmentID, (LLViewerJointAttachment*)NULL);

			if ( attachment_pt )
			{
				// make sure item is in your inventory (it could be a delayed attach message being sent from the sim)
				// so check to see if the item is in the inventory already
				item = gInventory.getItem(attachment_pt->getItemID());
				
				if ( !item )
				{
					// Item does not exist, make an observer to enable the pie menu 
					// when the item finishes fetching worst case scenario 
					// if a fetch is already out there (being sent from a slow sim)
					// we refetch and there are 2 fetches
					LLWornItemFetchedObserver* wornItemFetched = new LLWornItemFetchedObserver();
					LLInventoryFetchObserver::item_ref_t items; //add item to the inventory item to be fetched

					items.push_back(attachment_pt->getItemID());
				
					wornItemFetched->fetchItems(items);
					gInventory.addObserver(wornItemFetched);
				}
			}
		}
		
		//now check to make sure that the item is actually in the inventory before we enable dropping it
//		bool new_value = enable_detach(NULL) && can_build && item;
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
		bool new_value = enable_detach(NULL) && can_build && item && (!gRlvHandler.hasBehaviour(RLV_BHVR_REZ));
// [/RLVa:KB]
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

BOOL enable_detach(void*)
{
	LLViewerObject* object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
	if (!object) return FALSE;
	if (!object->isAttachment()) return FALSE;

	// Find the avatar who owns this attachment
	LLViewerObject* avatar = object;
	while (avatar)
	{
		// ...if it's you, good to detach
		if (avatar->getID() == gAgent.getID())
		{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
			// NOTE: this code is reused as-is in LLAttachmentDetach::handleEvent() and LLAttachmentDrop::handleEvent()
			//       so any changes here should be reflected there as well (I think it's in a number of other places as well by now)

			// RELEASE-RLVa: LLSelectMgr::sendDetach() and LLSelectMgr::sendDropAttachment() call sendListToRegions with
			//               SEND_ONLY_ROOTS so we only need to examine the roots which saves us time
			if ( (rlv_handler_t::isEnabled()) && (gRlvHandler.hasLockedAttachment()) )
			{
				LLObjectSelectionHandle hSelect = LLSelectMgr::getInstance()->getSelection();
				RlvSelectHasLockedAttach functor;
				if ( (hSelect->isAttachment()) && (hSelect->getFirstRootNode(&functor, FALSE)) )
					return FALSE;
			}
// [/RLVa:KB]
			return TRUE;
		}

		avatar = (LLViewerObject*)avatar->getParent();
	}

	return FALSE;
}

class LLAttachmentEnableDetach : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = enable_detach(NULL);
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

// Used to tell if the selected object can be attached to your avatar.
BOOL object_selected_and_point_valid(void *user_data)
{
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0b)
	if (rlv_handler_t::isEnabled())
	{
		// RELEASE-RLVa: look at the caller graph for this function on every new release
		//	-> 1.22.11 and 1.23.4
		//		- object_is_wearable() => dead code [user_data == NULL => default attach point => OK!]
		//      - LLObjectEnableWear::handleEvent() => Rezzed prim / right-click / "Wear" [user_data == NULL => see above]
		//      - enabler set up in LLVOAvatar::buildCharacter() => Rezzed prim / right-click / "Attach >" [user_data == pAttachPt]
		//      - enabler set up in LLVOAvatar::buildCharacter() => Rezzed prim / Edit menu / "Attach Object" [user_data == pAttachPt]
		LLViewerJointAttachment* pAttachPt = (LLViewerJointAttachment*)user_data;
		if  ( ((!pAttachPt) && (gRlvHandler.hasLockedAttachment())) ||		// Don't allow attach to default attach point
			  ((pAttachPt) && (!gRlvHandler.isDetachable(pAttachPt))) ||	// Don't allow replacing of locked attachment
			  (gRlvHandler.hasBehaviour(RLV_BHVR_REZ)) )					// Attaching a rezzed object == "Take"
		{
			return FALSE;
		}
	}
// [/RLVa:KB]

	//LLViewerJointAttachment *attachment = (LLViewerJointAttachment *)user_data;
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	for (LLObjectSelection::root_iterator iter = selection->root_begin();
		 iter != selection->root_end(); iter++)
	{
		LLSelectNode* node = *iter;
		LLViewerObject* object = node->getObject();
		LLViewerObject::const_child_list_t& child_list = object->getChildren();
		for (LLViewerObject::child_list_t::const_iterator iter = child_list.begin();
			 iter != child_list.end(); iter++)
		{
			LLViewerObject* child = *iter;
			if (child->isAvatar())
			{
				return FALSE;
			}
		}
	}

	return (selection->getRootObjectCount() == 1) && 
		(selection->getFirstRootObject()->getPCode() == LL_PCODE_VOLUME) && 
		selection->getFirstRootObject()->permYouOwner() &&
		!((LLViewerObject*)selection->getFirstRootObject()->getRoot())->isAvatar() && 
		(selection->getFirstRootObject()->getNVPair("AssetContainer") == NULL);
}


BOOL object_is_wearable()
{
	if (!object_selected_and_point_valid(NULL))
	{
		return FALSE;
	}
	if (sitting_on_selection())
	{
		return FALSE;
	}
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	for (LLObjectSelection::valid_root_iterator iter = LLSelectMgr::getInstance()->getSelection()->valid_root_begin();
		 iter != LLSelectMgr::getInstance()->getSelection()->valid_root_end(); iter++)
	{
		LLSelectNode* node = *iter;		
		if (node->mPermissions->getOwner() == gAgent.getID())
		{
			return TRUE;
		}
	}
	return FALSE;
}


// Also for seeing if object can be attached.  See above.
class LLObjectEnableWear : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool is_wearable = object_selected_and_point_valid(NULL);
		gMenuHolder->findControl(userdata["control"].asString())->setValue(is_wearable);
		return TRUE;
	}
};


BOOL object_attached(void *user_data)
{
	LLViewerJointAttachment *attachment = (LLViewerJointAttachment *)user_data;

// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
	return ( 
		      (attachment->getObject() != NULL) && 
			  ( (!rlv_handler_t::isEnabled()) || (gRlvHandler.isDetachable(attachment->getObject())) )
		   );
// [/RLVa:KB]
//	return attachment->getObject() != NULL;
}

class LLAvatarSendIM : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar* avatar = find_avatar_from_object( LLSelectMgr::getInstance()->getSelection()->getPrimaryObject() );
// [RLVa:KB] - Checked: 2009-07-08 (RLVa-1.0.0e)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
		{
			return true;
		}
// [/RLVa:KB]
		if(avatar)
		{
			std::string name("IM");
			LLNameValue *first = avatar->getNVPair("FirstName");
			LLNameValue *last = avatar->getNVPair("LastName");
			if (first && last)
			{
				name.assign( first->getString() );
				name.append(" ");
				name.append( last->getString() );
			}

			gIMMgr->setFloaterOpen(TRUE);
			//EInstantMessage type = have_agent_callingcard(gLastHitObjectID)
			//	? IM_SESSION_ADD : IM_SESSION_CARDLESS_START;
			gIMMgr->addSession(name,
								IM_NOTHING_SPECIAL,
								avatar->getID());
		}
		return true;
	}
};


void handle_activate(void*)
{
}

BOOL enable_activate(void*)
{
	return FALSE;
}

namespace
{
	struct QueueObjects : public LLSelectedObjectFunctor
	{
		BOOL scripted;
		BOOL modifiable;
		LLFloaterScriptQueue* mQueue;
		QueueObjects(LLFloaterScriptQueue* q) : mQueue(q), scripted(FALSE), modifiable(FALSE) {}
		virtual bool apply(LLViewerObject* obj)
		{
			scripted = obj->flagScripted();
			modifiable = obj->permModify();

			if( scripted && modifiable )
			{
				mQueue->addObject(obj->getID());
				return false;
			}
			else
			{
				return true; // fail: stop applying
			}
		}
	};
}

void queue_actions(LLFloaterScriptQueue* q, const std::string& noscriptmsg, const std::string& nomodmsg)
{
	QueueObjects func(q);
	LLSelectMgr *mgr = LLSelectMgr::getInstance();
	LLObjectSelectionHandle selectHandle = mgr->getSelection();
	bool fail = selectHandle->applyToObjects(&func);
	if(fail)
	{
		if ( !func.scripted )
		{
			gViewerWindow->alertXml(noscriptmsg);
		}
		else if ( !func.modifiable )
		{
			gViewerWindow->alertXml(nomodmsg);
		}
		else
		{
			llerrs << "Bad logic." << llendl;
		}
	}
	else
	{
		if (!q->start())
		{
			llwarns << "Unexpected script compile failure." << llendl;
		}
	}
}

class LLToolsSetBulkPerms : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLFloaterBulkPermission* queue = NULL;
		queue = LLFloaterBulkPermission::create();
		return true;
	}
};

void handle_compile_queue(std::string to_lang)
{
	LLFloaterCompileQueue* queue;
	if (to_lang == "mono")
	{
		queue = LLFloaterCompileQueue::create(TRUE);
	}
	else
	{
		queue = LLFloaterCompileQueue::create(FALSE);
	}
	queue_actions(queue, "CannotRecompileSelectObjectsNoScripts", "CannotRecompileSelectObjectsNoPermission");
}

void handle_reset_selection(void)
{
	LLFloaterResetQueue* queue = LLFloaterResetQueue::create();
	queue_actions(queue, "CannotResetSelectObjectsNoScripts", "CannotResetSelectObjectsNoPermission");
}

void handle_set_run_selection(void)
{
	LLFloaterRunQueue* queue = LLFloaterRunQueue::create();
	queue_actions(queue, "CannotSetRunningSelectObjectsNoScripts", "CannotSerRunningSelectObjectsNoPermission");
}

void handle_set_not_run_selection(void)
{
	LLFloaterNotRunQueue* queue = LLFloaterNotRunQueue::create();
	queue_actions(queue, "CannotSetRunningNotSelectObjectsNoScripts", "CannotSerRunningNotSelectObjectsNoPermission");
}

class LLToolsSelectedScriptAction : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
		if ( (rlv_handler_t::isEnabled()) && (gRlvHandler.hasLockedAttachment()) )
		{
			LLObjectSelectionHandle selectHandle = LLSelectMgr::getInstance()->getSelection();
			RlvSelectHasLockedAttach functor;
			if ( (selectHandle->isAttachment()) && (selectHandle->getFirstNode(&functor)) )
				return true;
		}
// [/RLVa:KB]

		std::string action = userdata.asString();
		if (action == "compile mono")
		{
			handle_compile_queue("mono");
		}
		if (action == "compile lsl")
		{
			handle_compile_queue("lsl");
		}
		else if (action == "reset")
		{
			handle_reset_selection();
		}
		else if (action == "start")
		{
			handle_set_run_selection();
		}
		else if (action == "stop")
		{
			handle_set_not_run_selection();
		}
		return true;
	}
};

void handle_selected_texture_info(void*)
{
	for (LLObjectSelection::valid_iterator iter = LLSelectMgr::getInstance()->getSelection()->valid_begin();
		 iter != LLSelectMgr::getInstance()->getSelection()->valid_end(); iter++)
	{
		LLSelectNode* node = *iter;
		
		std::string msg;
		msg.assign("Texture info for: ");
		msg.append(node->mName);
		LLChat chat(msg);
		LLFloaterChat::addChat(chat);

		U8 te_count = node->getObject()->getNumTEs();
		// map from texture ID to list of faces using it
		typedef std::map< LLUUID, std::vector<U8> > map_t;
		map_t faces_per_texture;
		for (U8 i = 0; i < te_count; i++)
		{
			if (!node->isTESelected(i)) continue;

			LLViewerImage* img = node->getObject()->getTEImage(i);
			LLUUID image_id = img->getID();
			faces_per_texture[image_id].push_back(i);
		}
		// Per-texture, dump which faces are using it.
		map_t::iterator it;
		for (it = faces_per_texture.begin(); it != faces_per_texture.end(); ++it)
		{
			LLUUID image_id = it->first;
			U8 te = it->second[0];
			LLViewerImage* img = node->getObject()->getTEImage(te);
			S32 height = img->getHeight();
			S32 width = img->getWidth();
			S32 components = img->getComponents();
			std::string image_id_string = image_id.asString();
			image_id_string = image_id_string.replace(24, 35, 12, '*') + " "; // hide last segment to discourage theft
			msg = llformat("%s%dx%d %s on face ",
								image_id_string.c_str(),
								width,
								height,
								(components == 4 ? "alpha" : "opaque"));
			for (U8 i = 0; i < it->second.size(); ++i)
			{
				msg.append( llformat("%d ", (S32)(it->second[i])));
			}
			LLChat chat(msg);
			LLFloaterChat::addChat(chat);
		}
	}
}

void handle_dump_image_list(void*)
{
	gImageList.dump();
}

void handle_test_male(void*)
{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
	if ( (rlv_handler_t::isEnabled()) && 
		 ( (gRlvHandler.hasLockedAttachment()) || 
		   (gRlvHandler.hasBehaviour(RLV_BHVR_ADDOUTFIT)) || (gRlvHandler.hasBehaviour(RLV_BHVR_REMOUTFIT)) ) )
	{
		return;
	}
// [/RLVa:KB]

	wear_outfit_by_name("Male Shape & Outfit");
	//gGestureList.requestResetFromServer( TRUE );
}

void handle_test_female(void*)
{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
	if ( (rlv_handler_t::isEnabled()) && 
		 ( (gRlvHandler.hasLockedAttachment()) || 
		   (gRlvHandler.hasBehaviour(RLV_BHVR_ADDOUTFIT)) || (gRlvHandler.hasBehaviour(RLV_BHVR_REMOUTFIT)) ) )
	{
		return;
	}
// [/RLVa:KB]

	wear_outfit_by_name("Female Shape & Outfit");
	//gGestureList.requestResetFromServer( FALSE );
}

void handle_toggle_pg(void*)
{
	gAgent.setTeen( !gAgent.isTeen() );

	LLFloaterWorldMap::reloadIcons(NULL);

	llinfos << "PG status set to " << (S32)gAgent.isTeen() << llendl;
}

void handle_dump_attachments(void*)
{
	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if( !avatar )
	{
		llinfos << "NO AVATAR" << llendl;
		return;
	}

	for (LLVOAvatar::attachment_map_t::iterator iter = avatar->mAttachmentPoints.begin(); 
		 iter != avatar->mAttachmentPoints.end(); )
	{
		LLVOAvatar::attachment_map_t::iterator curiter = iter++;
		LLViewerJointAttachment* attachment = curiter->second;
		S32 key = curiter->first;
		BOOL visible = (attachment->getObject() != NULL &&
						attachment->getObject()->mDrawable.notNull() && 
						!attachment->getObject()->mDrawable->isRenderType(0));
		LLVector3 pos;
		if (visible) pos = attachment->getObject()->mDrawable->getPosition();
		llinfos << "ATTACHMENT " << key << ": item_id=" << attachment->getItemID()
				<< (attachment->getObject() ? " present " : " absent ")
				<< (visible ? "visible " : "invisible ")
				<<  " at " << pos
				<< " and " << (visible ? attachment->getObject()->getPosition() : LLVector3::zero)
				<< llendl;
	}
}

//---------------------------------------------------------------------
// Callbacks for enabling/disabling items
//---------------------------------------------------------------------

BOOL menu_ui_enabled(void *user_data)
{
	BOOL high_res = gSavedSettings.getBOOL( "HighResSnapshot" );
	return !high_res;
}

// TomY TODO DEPRECATE & REMOVE
void menu_toggle_control( void* user_data )
{
        BOOL checked = gSavedSettings.getBOOL( static_cast<char*>(user_data) );
        gSavedSettings.setBOOL( static_cast<char*>(user_data), !checked );
}


// these are used in the gl menus to set control values.
class LLToggleControl : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string control_name = userdata.asString();
		BOOL checked = gSavedSettings.getBOOL( control_name );
		gSavedSettings.setBOOL( control_name, !checked );
		return true;
	}
};

// As above, but can be a callback from a LLCheckboxCtrl
void check_toggle_control( LLUICtrl *, void* user_data )
{
	BOOL checked = gSavedSettings.getBOOL( static_cast<char*>(user_data) );
	gSavedSettings.setBOOL( static_cast<char*>(user_data), !checked );
}

BOOL menu_check_control( void* user_data)
{
	return gSavedSettings.getBOOL((char*)user_data);
}

// 
void menu_toggle_variable( void* user_data )
{
	BOOL checked = *(BOOL*)user_data;
	*(BOOL*)user_data = !checked;
}

BOOL menu_check_variable( void* user_data)
{
	return *(BOOL*)user_data;
}


BOOL enable_land_selected( void* )
{
	return !(LLViewerParcelMgr::getInstance()->selectionEmpty());
}

void menu_toggle_attached_lights(void* user_data)
{
	menu_toggle_control(user_data);
	LLPipeline::sRenderAttachedLights = gSavedSettings.getBOOL("RenderAttachedLights");
}

void menu_toggle_attached_particles(void* user_data)
{
	menu_toggle_control(user_data);
	LLPipeline::sRenderAttachedParticles = gSavedSettings.getBOOL("RenderAttachedParticles");
}

class LLSomethingSelected : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = !(LLSelectMgr::getInstance()->getSelection()->isEmpty());
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLSomethingSelectedNoHUD : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
		bool new_value = !(selection->isEmpty()) && !(selection->getSelectType() == SELECT_TYPE_HUD);
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

BOOL enable_more_than_one_selected(void* )
{
	return (LLSelectMgr::getInstance()->getSelection()->getObjectCount() > 1);
}

static bool is_editable_selected()
{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c) | Modified: RLVa-1.0.0c
	// RELEASE-RLVa: check that this still isn't called by anything but script actions in the Tools menu
	if ( (rlv_handler_t::isEnabled()) && (gRlvHandler.hasLockedAttachment()) )
	{
		LLObjectSelectionHandle selectHandle = LLSelectMgr::getInstance()->getSelection();

		// NOTE: this is called for 5 different menu items so we'll trade accuracy for efficiency and only
		//       examine root nodes (LLToolsSelectedScriptAction::handleEvent() will catch what we miss)
		RlvSelectHasLockedAttach functor;
		if ( (selectHandle->isAttachment()) && (selectHandle->getFirstRootNode(&functor)) )
		{
			return false;
		}
	}
// [/RLVa:KB]

	return (LLSelectMgr::getInstance()->getSelection()->getFirstEditableObject() != NULL);
}

class LLEditableSelected : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gMenuHolder->findControl(userdata["control"].asString())->setValue(is_editable_selected());
		return true;
	}
};

class LLEditableSelectedMono : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerRegion* region = gAgent.getRegion();
		if(region && gMenuHolder && gMenuHolder->findControl(userdata["control"].asString()))
		{
			bool have_cap = (! region->getCapability("UpdateScriptTask").empty());
			bool selected = is_editable_selected() && have_cap;
			gMenuHolder->findControl(userdata["control"].asString())->setValue(selected);
			return true;
		}
		return false;
	}
};

class LLToolsEnableTakeCopy : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool all_valid = false;
		if (LLSelectMgr::getInstance())
		{
			all_valid = true;
#ifndef HACKED_GODLIKE_VIEWER
# ifdef TOGGLE_HACKED_GODLIKE_VIEWER
			if (LLViewerLogin::getInstance()->isInProductionGrid()
                || !gAgent.isGodlike())
# endif
			{
				struct f : public LLSelectedObjectFunctor
				{
					virtual bool apply(LLViewerObject* obj)
					{
//						return (!obj->permCopy() || obj->isAttachment());
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
						return (!obj->permCopy() || obj->isAttachment()) || 
							   ( (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) && (gAgent.getAvatarObject()) && 
							     (gAgent.getAvatarObject()->getRoot() == obj) );
// [/RLVa:KB]
					}
				} func;
				const bool firstonly = true;
				bool any_invalid = LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func, firstonly);
				all_valid = !any_invalid;
			}
#endif // HACKED_GODLIKE_VIEWER
		}

		gMenuHolder->findControl(userdata["control"].asString())->setValue(all_valid);
		return true;
	}
};

BOOL enable_selection_you_own_all(void*)
{
	if (LLSelectMgr::getInstance())
	{
		struct f : public LLSelectedObjectFunctor
		{
			virtual bool apply(LLViewerObject* obj)
			{
				return (!obj->permYouOwner());
			}
		} func;
		const bool firstonly = true;
		bool no_perms = LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func, firstonly);
		if (no_perms)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL enable_selection_you_own_one(void*)
{
	if (LLSelectMgr::getInstance())
	{
		struct f : public LLSelectedObjectFunctor
		{
			virtual bool apply(LLViewerObject* obj)
			{
				return (obj->permYouOwner());
			}
		} func;
		const bool firstonly = true;
		bool any_perms = LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func, firstonly);
		if (!any_perms)
		{
			return FALSE;
		}
	}
	return TRUE;
}

class LLHasAsset : public LLInventoryCollectFunctor
{
public:
	LLHasAsset(const LLUUID& id) : mAssetID(id), mHasAsset(FALSE) {}
	virtual ~LLHasAsset() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
	BOOL hasAsset() const { return mHasAsset; }

protected:
	LLUUID mAssetID;
	BOOL mHasAsset;
};

bool LLHasAsset::operator()(LLInventoryCategory* cat,
							LLInventoryItem* item)
{
	if(item && item->getAssetUUID() == mAssetID)
	{
		mHasAsset = TRUE;
	}
	return FALSE;
}

BOOL enable_save_into_inventory(void*)
{
	// *TODO: clean this up
	// find the last root
	LLSelectNode* last_node = NULL;
	for (LLObjectSelection::root_iterator iter = LLSelectMgr::getInstance()->getSelection()->root_begin();
		 iter != LLSelectMgr::getInstance()->getSelection()->root_end(); iter++)
	{
		last_node = *iter;
	}

#ifdef HACKED_GODLIKE_VIEWER
	return TRUE;
#else
# ifdef TOGGLE_HACKED_GODLIKE_VIEWER
	if (!LLViewerLogin::getInstance()->isInProductionGrid()
        && gAgent.isGodlike())
	{
		return TRUE;
	}
# endif
	// check all pre-req's for save into inventory.
	if(last_node && last_node->mValid && !last_node->mItemID.isNull()
	   && (last_node->mPermissions->getOwner() == gAgent.getID())
	   && (gInventory.getItem(last_node->mItemID) != NULL))
	{
		LLViewerObject* obj = last_node->getObject();
		if( obj && !obj->isAttachment() )
		{
			return TRUE;
		}
	}
#endif
	return FALSE;
}

class LLToolsEnableSaveToInventory : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = enable_save_into_inventory(NULL);
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

BOOL enable_save_into_task_inventory(void*)
{
	LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode();
	if(node && (node->mValid) && (!node->mFromTaskID.isNull()))
	{
		// *TODO: check to see if the fromtaskid object exists.
		LLViewerObject* obj = node->getObject();
		if( obj && !obj->isAttachment() )
		{
			return TRUE;
		}
	}
	return FALSE;
}

class LLToolsEnableSaveToObjectInventory : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = enable_save_into_task_inventory(NULL);
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

BOOL enable_not_thirdperson(void*)
{
	return !gAgent.cameraThirdPerson();
}


// BOOL enable_export_selected(void *)
// {
// 	if (LLSelectMgr::getInstance()->getSelection()->isEmpty())
// 	{
// 		return FALSE;
// 	}
// 	if (!gExporterRequestID.isNull())
// 	{
// 		return FALSE;
// 	}
// 	if (!LLUploadDialog::modalUploadIsFinished())
// 	{
// 		return FALSE;
// 	}
// 	return TRUE;
// }

class LLViewEnableMouselook : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// You can't go directly from customize avatar to mouselook.
		// TODO: write code with appropriate dialogs to handle this transition.
		bool new_value = (CAMERA_MODE_CUSTOMIZE_AVATAR != gAgent.getCameraMode() && !gSavedSettings.getBOOL("FreezeTime"));
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLToolsEnableToolNotPie : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = ( LLToolMgr::getInstance()->getBaseTool() != LLToolPie::getInstance() );
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLWorldEnableCreateLandmark : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gAgent.isGodlike() || 
			(gAgent.getRegion() && gAgent.getRegion()->getAllowLandmark());
// [RLVa:KB] - Checked: 2009-07-04 (RLVa-1.0.0a)
		new_value &= !gRlvHandler.hasBehaviour(RLV_BHVR_SHOWLOC);
// [/RLVa:KB]
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLWorldEnableSetHomeLocation : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gAgent.isGodlike() || 
			(gAgent.getRegion() && gAgent.getRegion()->getAllowSetHome());
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLWorldEnableTeleportHome : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerRegion* regionp = gAgent.getRegion();
		bool agent_on_prelude = (regionp && regionp->isPrelude());
		bool enable_teleport_home = gAgent.isGodlike() || !agent_on_prelude;
		gMenuHolder->findControl(userdata["control"].asString())->setValue(enable_teleport_home);
		return true;
	}
};

BOOL enable_region_owner(void*)
{
	if(gAgent.getRegion() && gAgent.getRegion()->getOwner() == gAgent.getID())
		return TRUE;
	return enable_god_customer_service(NULL);
}

BOOL enable_god_full(void*)
{
	return gAgent.getGodLevel() >= GOD_FULL;
}

BOOL enable_god_liaison(void*)
{
	return gAgent.getGodLevel() >= GOD_LIAISON;
}

BOOL enable_god_customer_service(void*)
{
	return gAgent.getGodLevel() >= GOD_CUSTOMER_SERVICE;
}

BOOL enable_god_basic(void*)
{
// [RLVa:KB] - Checked: 2009-07-04 (RLVa-1.0.0a)
	// RELEASE-RLVa: check that this function isn't used for anything other than to enable/disable showing the "God Tools..." floater
	return (gAgent.getGodLevel() > GOD_NOT) && (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWLOC));
// [/RLVa:KB]
	//return gAgent.getGodLevel() > GOD_NOT;
}

#if 0 // 1.9.2
void toggle_vertex_shaders(void *)
{
	BOOL use_shaders = gPipeline.getUseVertexShaders();
	gPipeline.setUseVertexShaders(use_shaders);
}

BOOL check_vertex_shaders(void *)
{
	return gPipeline.getUseVertexShaders();
}
#endif

void toggle_show_xui_names(void *)
{
	BOOL showXUINames = gSavedSettings.getBOOL("ShowXUINames");
	
	showXUINames = !showXUINames;
	gSavedSettings.setBOOL("ShowXUINames", showXUINames);
}

BOOL check_show_xui_names(void *)
{
	return gSavedSettings.getBOOL("ShowXUINames");
}



void toggle_cull_small(void *)
{
//	gPipeline.mCullBySize = !gPipeline.mCullBySize;
//
//	gSavedSettings.setBOOL("RenderCullBySize", gPipeline.mCullBySize);
}

class LLToolsSelectOnlyMyObjects : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		BOOL cur_val = gSavedSettings.getBOOL("SelectOwnedOnly");

		gSavedSettings.setBOOL("SelectOwnedOnly", ! cur_val );

		return true;
	}
};

class LLToolsSelectOnlyMovableObjects : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		BOOL cur_val = gSavedSettings.getBOOL("SelectMovableOnly");

		gSavedSettings.setBOOL("SelectMovableOnly", ! cur_val );

		return true;
	}
};

class LLToolsSelectOnlyCopyableObjects : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		BOOL cur_val = gSavedSettings.getBOOL("SelectCopyableOnly");

		gSavedSettings.setBOOL("SelectCopyableOnly", ! cur_val );

		return true;
	}
};

class LLToolsSelectBySurrounding : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLSelectMgr::sRectSelectInclusive = !LLSelectMgr::sRectSelectInclusive;

		gSavedSettings.setBOOL("RectangleSelectInclusive", LLSelectMgr::sRectSelectInclusive);
		return true;
	}
};

class LLToolsShowSelectionHighlights : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLSelectMgr::sRenderSelectionHighlights = !LLSelectMgr::sRenderSelectionHighlights;
		
		gSavedSettings.setBOOL("RenderHighlightSelections", LLSelectMgr::sRenderSelectionHighlights);
		return true;
	}
};

class LLToolsShowHiddenSelection : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// TomY TODO Merge these
		LLSelectMgr::sRenderHiddenSelections = !LLSelectMgr::sRenderHiddenSelections;

		gSavedSettings.setBOOL("RenderHiddenSelections", LLSelectMgr::sRenderHiddenSelections);
		return true;
	}
};

class LLToolsShowSelectionLightRadius : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// TomY TODO merge these
		LLSelectMgr::sRenderLightRadius = !LLSelectMgr::sRenderLightRadius;

		gSavedSettings.setBOOL("RenderLightRadius", LLSelectMgr::sRenderLightRadius);
		return true;
	}
};

class LLToolsEditLinkedParts : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		BOOL select_individuals = gSavedSettings.getBOOL("EditLinkedParts");
		if (select_individuals)
		{
			LLSelectMgr::getInstance()->demoteSelectionToIndividuals();
		}
		else
		{
			LLSelectMgr::getInstance()->promoteSelectionToRoot();
		}
		return true;
	}
};

void reload_personal_settings_overrides(void *)
{
	llinfos << "Loading overrides from " << gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT,"overrides.xml") << llendl;
	
	gSavedSettings.loadFromFile(gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT,"overrides.xml"));
}

void reload_vertex_shader(void *)
{
	//THIS WOULD BE AN AWESOME PLACE TO RELOAD SHADERS... just a thought	- DaveP
}

void slow_mo_animations(void*)
{
	static BOOL slow_mo = FALSE;
	if (slow_mo)
	{
		gAgent.getAvatarObject()->setAnimTimeFactor(1.f);
		slow_mo = FALSE;
	}
	else
	{
		gAgent.getAvatarObject()->setAnimTimeFactor(0.2f);
		slow_mo = TRUE;
	}
}

void handle_dump_avatar_local_textures(void*)
{
	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if( avatar )
	{
		avatar->dumpLocalTextures();
	}
}

void handle_debug_avatar_textures(void*)
{
	LLViewerObject* objectp = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
	if (objectp)
	{
		LLFloaterAvatarTextures::show(objectp->getID());
	}
}

void handle_grab_texture(void* data)
{
	LLVOAvatar::ETextureIndex index = (LLVOAvatar::ETextureIndex)((intptr_t)data);
	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if ( avatar )
	{
		const LLUUID& asset_id = avatar->grabLocalTexture(index);
		llinfos << "Adding baked texture " << asset_id << " to inventory." << llendl;
		LLAssetType::EType asset_type = LLAssetType::AT_TEXTURE;
		LLInventoryType::EType inv_type = LLInventoryType::IT_TEXTURE;
		LLUUID folder_id(gInventory.findCategoryUUIDForType(asset_type));
		if(folder_id.notNull())
		{
			std::string name = "Baked ";
			switch (index)
			{
			case LLVOAvatar::TEX_EYES_BAKED:
				name.append("Iris");
				break;
			case LLVOAvatar::TEX_HEAD_BAKED:
				name.append("Head");
				break;
			case LLVOAvatar::TEX_UPPER_BAKED:
				name.append("Upper Body");
				break;
			case LLVOAvatar::TEX_LOWER_BAKED:
				name.append("Lower Body");
				break;
			case LLVOAvatar::TEX_SKIRT_BAKED:
				name.append("Skirt");
				break;
			default:
				name.append("Unknown");
				break;
			}
			name.append(" Texture");

			LLUUID item_id;
			item_id.generate();
			LLPermissions perm;
			perm.init(gAgentID,
					  gAgentID,
					  LLUUID::null,
					  LLUUID::null);
			U32 next_owner_perm = PERM_MOVE | PERM_TRANSFER;
			perm.initMasks(PERM_ALL,
						   PERM_ALL,
						   PERM_NONE,
						   PERM_NONE,
						   next_owner_perm);
			time_t creation_date_now = time_corrected();
			LLPointer<LLViewerInventoryItem> item
				= new LLViewerInventoryItem(item_id,
											folder_id,
											perm,
											asset_id,
											asset_type,
											inv_type,
											name,
											LLStringUtil::null,
											LLSaleInfo::DEFAULT,
											LLInventoryItem::II_FLAGS_NONE,
											creation_date_now);

			item->updateServer(TRUE);
			gInventory.updateItem(item);
			gInventory.notifyObservers();

			LLInventoryView* view = LLInventoryView::getActiveInventory();

			// Show the preview panel for textures to let
			// user know that the image is now in inventory.
			if(view)
			{
				LLUICtrl* focus_ctrl = gFocusMgr.getKeyboardFocus();

				view->getPanel()->setSelection(item_id, TAKE_FOCUS_NO);
				view->getPanel()->openSelected();
				//LLInventoryView::dumpSelectionInformation((void*)view);
				// restore keyboard focus
				gFocusMgr.setKeyboardFocus(focus_ctrl);
			}
		}
		else
		{
			llwarns << "Can't find a folder to put it in" << llendl;
		}
	}
}

BOOL enable_grab_texture(void* data)
{
	LLVOAvatar::ETextureIndex index = (LLVOAvatar::ETextureIndex)((intptr_t)data);
	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if ( avatar )
	{
		return avatar->canGrabLocalTexture(index);
	}
	return FALSE;
}

// Returns a pointer to the avatar give the UUID of the avatar OR of an attachment the avatar is wearing.
// Returns NULL on failure.
LLVOAvatar* find_avatar_from_object( LLViewerObject* object )
{
	if (object)
	{
		if( object->isAttachment() )
		{
			do
			{
				object = (LLViewerObject*) object->getParent();
			}
			while( object && !object->isAvatar() );
		}
		else
		if( !object->isAvatar() )
		{
			object = NULL;
		}
	}

	return (LLVOAvatar*) object;
}


// Returns a pointer to the avatar give the UUID of the avatar OR of an attachment the avatar is wearing.
// Returns NULL on failure.
LLVOAvatar* find_avatar_from_object( const LLUUID& object_id )
{
	return find_avatar_from_object( gObjectList.findObject(object_id) );
}


void handle_disconnect_viewer(void *)
{
	LLAppViewer::instance()->forceDisconnect("Testing viewer disconnect");
}

void force_error_breakpoint(void *)
{
    LLAppViewer::instance()->forceErrorBreakpoint();
}

void force_error_llerror(void *)
{
    LLAppViewer::instance()->forceErrorLLError();
}

void force_error_bad_memory_access(void *)
{
    LLAppViewer::instance()->forceErrorBadMemoryAccess();
}

void force_error_infinite_loop(void *)
{
    LLAppViewer::instance()->forceErrorInifiniteLoop();
}

void force_error_software_exception(void *)
{
    LLAppViewer::instance()->forceErrorSoftwareException();
}

void force_error_driver_crash(void *)
{
    LLAppViewer::instance()->forceErrorDriverCrash();
}

class LLToolsUseSelectionForGrid : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLSelectMgr::getInstance()->clearGridObjects();
		struct f : public LLSelectedObjectFunctor
		{
			virtual bool apply(LLViewerObject* objectp)
			{
				LLSelectMgr::getInstance()->addGridObject(objectp);
				return true;
			}
		} func;
		LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func);
		LLSelectMgr::getInstance()->setGridMode(GRID_MODE_REF_OBJECT);
		if (gFloaterTools)
		{
			gFloaterTools->mComboGridMode->setCurrentByIndex((S32)GRID_MODE_REF_OBJECT);
		}
		return true;
	}
};

void handle_test_load_url(void*)
{
	LLWeb::loadURL("");
	LLWeb::loadURL("hacker://www.google.com/");
	LLWeb::loadURL("http");
	LLWeb::loadURL("http://www.google.com/");
}

//
// LLViewerMenuHolderGL
//

LLViewerMenuHolderGL::LLViewerMenuHolderGL() : LLMenuHolderGL()
{
}

BOOL LLViewerMenuHolderGL::hideMenus()
{
	BOOL handled = LLMenuHolderGL::hideMenus();

	// drop pie menu selection
	mParcelSelection = NULL;
	mObjectSelection = NULL;

	gMenuBarView->clearHoverItem();
	gMenuBarView->resetMenuTrigger();

	return handled;
}

void LLViewerMenuHolderGL::setParcelSelection(LLSafeHandle<LLParcelSelection> selection) 
{ 
	mParcelSelection = selection; 
}

void LLViewerMenuHolderGL::setObjectSelection(LLSafeHandle<LLObjectSelection> selection) 
{ 
	mObjectSelection = selection; 
}


const LLRect LLViewerMenuHolderGL::getMenuRect() const
{
	return LLRect(0, getRect().getHeight() - MENU_BAR_HEIGHT, getRect().getWidth(), STATUS_BAR_HEIGHT);
}

void handle_save_to_xml(void*)
{
	LLFloater* frontmost = gFloaterView->getFrontmost();
	if (!frontmost)
	{
        gViewerWindow->alertXml("NoFrontmostFloater");
		return;
	}

	std::string default_name = "floater_";
	default_name += frontmost->getTitle();
	default_name += ".xml";

	LLStringUtil::toLower(default_name);
	LLStringUtil::replaceChar(default_name, ' ', '_');
	LLStringUtil::replaceChar(default_name, '/', '_');
	LLStringUtil::replaceChar(default_name, ':', '_');
	LLStringUtil::replaceChar(default_name, '"', '_');

	LLFilePicker& picker = LLFilePicker::instance();
	if (picker.getSaveFile(LLFilePicker::FFSAVE_XML, default_name))
	{
		std::string filename = picker.getFirstFile();
		LLUICtrlFactory::getInstance()->saveToXML(frontmost, filename);
	}
}

void handle_load_from_xml(void*)
{
	LLFilePicker& picker = LLFilePicker::instance();
	if (picker.getOpenFile(LLFilePicker::FFLOAD_XML))
	{
		std::string filename = picker.getFirstFile();
		LLFloater* floater = new LLFloater("sample_floater");
		LLUICtrlFactory::getInstance()->buildFloater(floater, filename);
	}
}

void handle_slurl_test(void*)
{
	std::string test_slurl = "http://secondlife.com/app/search/slurls.html";
	LLFloaterMediaBrowser::showInstance(test_slurl);
}

void handle_rebake_textures(void*)
{
	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if (!avatar) return;

	// Slam pending upload count to "unstick" things
	bool slam_for_debug = true;
	avatar->forceBakeAllTextures(slam_for_debug);
}

void toggle_visibility(void* user_data)
{
	LLView* viewp = (LLView*)user_data;
	viewp->setVisible(!viewp->getVisible());
}

BOOL get_visibility(void* user_data)
{
	LLView* viewp = (LLView*)user_data;
	return viewp->getVisible();
}

// TomY TODO: Get rid of these?
class LLViewShowHoverTips : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLHoverView::sShowHoverTips = !LLHoverView::sShowHoverTips;
		return true;
	}
};

class LLViewCheckShowHoverTips : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLHoverView::sShowHoverTips;
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

// TomY TODO: Get rid of these?
class LLViewHighlightTransparent : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-04 (RLVa-1.0.0b)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_EDIT))
		{
			return true;
		}
// [/RLVa:KB]

		LLDrawPoolAlpha::sShowDebugAlpha = !LLDrawPoolAlpha::sShowDebugAlpha;
		return true;
	}
};

class LLViewCheckHighlightTransparent : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLDrawPoolAlpha::sShowDebugAlpha;
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLViewToggleRenderType : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string type = userdata.asString();
		if (type == "hideparticles")
		{
			LLPipeline::toggleRenderType(LLPipeline::RENDER_TYPE_PARTICLES);
		}
		return true;
	}
};

class LLViewCheckRenderType : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string type = userdata["data"].asString();
		bool new_value = false;
		if (type == "hideparticles")
		{
			new_value = LLPipeline::toggleRenderTypeControlNegated((void *)LLPipeline::RENDER_TYPE_PARTICLES);
		}
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLViewShowHUDAttachments : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.0.0c)
		if ( (LLPipeline::sShowHUDAttachments) && (rlv_handler_t::isEnabled()) && (gRlvHandler.hasLockedHUD()) )
		{
			return true;
		}
// [/RLVa:KB]

		LLPipeline::sShowHUDAttachments = !LLPipeline::sShowHUDAttachments;
		return true;
	}
};

class LLViewCheckHUDAttachments : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLPipeline::sShowHUDAttachments;
		gMenuHolder->findControl(userdata["control"].asString())->setValue(new_value);
		return true;
	}
};

class LLEditEnableTakeOff : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string control_name = userdata["control"].asString();
		std::string clothing = userdata["data"].asString();
		bool new_value = false;
		if (clothing == "shirt")
		{
			new_value = LLAgent::selfHasWearable((void *)WT_SHIRT);
		}
		if (clothing == "pants")
		{
			new_value = LLAgent::selfHasWearable((void *)WT_PANTS);
		}
		if (clothing == "shoes")
		{
			new_value = LLAgent::selfHasWearable((void *)WT_SHOES);
		}
		if (clothing == "socks")
		{
			new_value = LLAgent::selfHasWearable((void *)WT_SOCKS);
		}
		if (clothing == "jacket")
		{
			new_value = LLAgent::selfHasWearable((void *)WT_JACKET);
		}
		if (clothing == "gloves")
		{
			new_value = LLAgent::selfHasWearable((void *)WT_GLOVES);
		}
		if (clothing == "undershirt")
		{
			new_value = LLAgent::selfHasWearable((void *)WT_UNDERSHIRT);
		}
		if (clothing == "underpants")
		{
			new_value = LLAgent::selfHasWearable((void *)WT_UNDERPANTS);
		}
		if (clothing == "skirt")
		{
			new_value = LLAgent::selfHasWearable((void *)WT_SKIRT);
		}

// [RLVa:KB] - Checked: 2009-07-07 (RLVa-1.0.0d)
		// Why aren't they using LLWearable::typeNameToType()? *confuzzled*
		if ( (rlv_handler_t::isEnabled()) && (!gRlvHandler.isRemovable(LLWearable::typeNameToType(clothing))) )
		{
			new_value = false;
		}
// [/RLVa:KB]

		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};

class LLEditTakeOff : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string clothing = userdata.asString();
		if (clothing == "shirt")
		{
			LLAgent::userRemoveWearable((void*)WT_SHIRT);
		}
		else if (clothing == "pants")
		{
			LLAgent::userRemoveWearable((void*)WT_PANTS);
		}
		else if (clothing == "shoes")
		{
			LLAgent::userRemoveWearable((void*)WT_SHOES);
		}
		else if (clothing == "socks")
		{
			LLAgent::userRemoveWearable((void*)WT_SOCKS);
		}
		else if (clothing == "jacket")
		{
			LLAgent::userRemoveWearable((void*)WT_JACKET);
		}
		else if (clothing == "gloves")
		{
			LLAgent::userRemoveWearable((void*)WT_GLOVES);
		}
		else if (clothing == "undershirt")
		{
			LLAgent::userRemoveWearable((void*)WT_UNDERSHIRT);
		}
		else if (clothing == "underpants")
		{
			LLAgent::userRemoveWearable((void*)WT_UNDERPANTS);
		}
		else if (clothing == "skirt")
		{
			LLAgent::userRemoveWearable((void*)WT_SKIRT);
		}
		else if (clothing == "all")
		{
			LLAgent::userRemoveAllClothesConfirm();
		}
		return true;
	}
};

class LLWorldChat : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_chat(NULL);
		return true;
	}
};

class LLToolsSelectTool : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string tool_name = userdata.asString();
		if (tool_name == "focus")
		{
			LLToolMgr::getInstance()->getCurrentToolset()->selectToolByIndex(1);
		}
		else if (tool_name == "move")
		{
			LLToolMgr::getInstance()->getCurrentToolset()->selectToolByIndex(2);
		}
		else if (tool_name == "edit")
		{
			LLToolMgr::getInstance()->getCurrentToolset()->selectToolByIndex(3);
		}
		else if (tool_name == "create")
		{
			LLToolMgr::getInstance()->getCurrentToolset()->selectToolByIndex(4);
		}
		else if (tool_name == "land")
		{
			LLToolMgr::getInstance()->getCurrentToolset()->selectToolByIndex(5);
		}
		return true;
	}
};

/// WINDLIGHT callbacks
class LLWorldEnvSettings : public view_listener_t
{	
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SETENV))
		{
			return true;
		}
// [/RLVa:KB]

		std::string tod = userdata.asString();
		LLVector3 sun_direction;
		
		if (tod == "editor")
		{
			// if not there or is hidden, show it
			if(	!LLFloaterEnvSettings::isOpen() || 
				!LLFloaterEnvSettings::instance()->getVisible()) {
				LLFloaterEnvSettings::show();
				
			// otherwise, close it button acts like a toggle
			} 
			else 
			{
				LLFloaterEnvSettings::instance()->close();
			}
			return true;
		}
		
		if (tod == "sunrise")
		{
			// set the value, turn off animation
			LLWLParamManager::instance()->mAnimator.setDayTime(0.25);
			LLWLParamManager::instance()->mAnimator.mIsRunning = false;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = false;

			// then call update once
			LLWLParamManager::instance()->mAnimator.update(
				LLWLParamManager::instance()->mCurParams);
		}
		else if (tod == "noon")
		{
			// set the value, turn off animation
			LLWLParamManager::instance()->mAnimator.setDayTime(0.567);
			LLWLParamManager::instance()->mAnimator.mIsRunning = false;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = false;

			// then call update once
			LLWLParamManager::instance()->mAnimator.update(
				LLWLParamManager::instance()->mCurParams);
		}
		else if (tod == "sunset")
		{
			// set the value, turn off animation
			LLWLParamManager::instance()->mAnimator.setDayTime(0.75);
			LLWLParamManager::instance()->mAnimator.mIsRunning = false;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = false;

			// then call update once
			LLWLParamManager::instance()->mAnimator.update(
				LLWLParamManager::instance()->mCurParams);
		}
		else if (tod == "midnight")
		{
			// set the value, turn off animation
			LLWLParamManager::instance()->mAnimator.setDayTime(0.0);
			LLWLParamManager::instance()->mAnimator.mIsRunning = false;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = false;

			// then call update once
			LLWLParamManager::instance()->mAnimator.update(
				LLWLParamManager::instance()->mCurParams);
		}
		else
		{
			LLWLParamManager::instance()->mAnimator.mIsRunning = true;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = true;	
		}
		return true;
	}
};

/// Water Menu callbacks
class LLWorldWaterSettings : public view_listener_t
{	
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SETENV))
		{
			return true;
		}
// [/RLVa:KB]

		// if not there or is hidden, show it
		if(	!LLFloaterWater::isOpen() || 
			!LLFloaterWater::instance()->getVisible()) {
			LLFloaterWater::show();
				
		// otherwise, close it button acts like a toggle
		} 
		else 
		{
			LLFloaterWater::instance()->close();
		}
		return true;
	}
};

/// Post-Process callbacks
class LLWorldPostProcess : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLFloaterPostProcess::show();
		return true;
	}
};

/// Day Cycle callbacks
class LLWorldDayCycle : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SETENV))
		{
			return true;
		}
// [/RLVa:KB]

		LLFloaterDayCycle::show();
		return true;
	}
};



//-------------------------------------------------------------------
// Advanced menu
//-------------------------------------------------------------------


///////////////////
// SHOW CONSOLES //
///////////////////


class LLAdvancedToggleConsole : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string console_type = userdata.asString();
		if ("frame" == console_type)
		{
			toggle_visibility( (void*)gDebugView->mFrameStatView );
		}
		else if ("texture" == console_type)
		{
			toggle_visibility( (void*)gTextureView );
		}
		else if ("debug" == console_type)
		{
			toggle_visibility( (void*)((LLView*)gDebugView->mDebugConsolep) );
		}
		else if ("fast timers" == console_type)
		{
			toggle_visibility( (void*)gDebugView->mFastTimerView );
		}
		else if ("memory" == console_type)
		{
			toggle_visibility( (void*)gDebugView->mMemoryView );
		}
		return true;
	}
};


class LLAdvancedCheckConsole : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string console_type = userdata["data"].asString();
		bool new_value = false;
		if ("frame" == console_type)
		{
			new_value = get_visibility( (void*)gDebugView->mFrameStatView );
		}
		else if ("texture" == console_type)
		{
			new_value = get_visibility( (void*)gTextureView );
		}
		else if ("debug" == console_type)
		{
			new_value = get_visibility( (void*)((LLView*)gDebugView->mDebugConsolep) );
		}
		else if ("fast timers" == console_type)
		{
			new_value = get_visibility( (void*)gDebugView->mFastTimerView );
		}
		else if ("memory" == console_type)
		{
			new_value = get_visibility( (void*)gDebugView->mMemoryView );
		}

		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



//////////////////////////
// DUMP INFO TO CONSOLE //
//////////////////////////


class LLAdvancedDumpInfoToConsole : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string info_type = userdata.asString();
		if ("region" == info_type)
		{
			handle_region_dump_settings(NULL);
		}
		else if ("group" == info_type)
		{
			handle_dump_group_info(NULL);
		}
		else if ("capabilities" == info_type)
		{
			handle_dump_capabilities_info(NULL);
		}
		return true;
	}
};



///////////////////////////////
// RELOAD SETTINGS OVERRIDES //
///////////////////////////////


class LLAdvancedReloadSettingsOverrides : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		reload_personal_settings_overrides(NULL);
		return true;
	}
};



//////////////
// HUD INFO //
//////////////


class LLAdvancedToggleHUDInfo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string info_type = userdata.asString();
		if ("velocity" == info_type)
		{
			toggle_visibility( (void*)gVelocityBar );
		}
		else if ("camera" == info_type)
		{
			gDisplayCameraPos = !(gDisplayCameraPos);
		}
		else if ("wind" == info_type)
		{
			gDisplayWindInfo = !(gDisplayWindInfo);
		}
		else if ("fov" == info_type)
		{
			gDisplayFOV = !(gDisplayFOV);
		}
		return true;
	}
};

class LLAdvancedCheckHUDInfo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string info_type = userdata["data"].asString();
		bool new_value = false;
		if ("velocity" == info_type)
		{
			new_value = get_visibility( (void*)gVelocityBar );
		}
		else if ("camera" == info_type)
		{
			new_value = gDisplayCameraPos;
		}
		else if ("wind" == info_type)
		{
			new_value = gDisplayWindInfo;
		}
		else if ("fov" == info_type)
		{
			new_value = gDisplayFOV;
		}

		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		
		return true;
	}
};


//////////////////////
// FORCE GROUND SIT //
//////////////////////

class LLAdvancedToggleSit: public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLChat chat;
		chat.mSourceType = CHAT_SOURCE_SYSTEM;
		if(!gAgent.getAvatarObject()->mIsSitting)
		{
			gAgent.setControlFlags(AGENT_CONTROL_SIT_ON_GROUND);
			chat.mText = "Forcing Ground Sit";
		}
		else
		{
			gAgent.setControlFlags(!AGENT_CONTROL_SIT_ON_GROUND);
			gAgent.setControlFlags(AGENT_CONTROL_STAND_UP);
			chat.mText = "Standing up";
		}
		LLFloaterChat::addChat(chat);
		return true;
	}
};

class LLAdvancedCheckSit : public view_listener_t
{
    bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
    {
		if(gAgent.getAvatarObject()->mIsSitting)
		{
			gMenuHolder->findControl(userdata["control"].asString())->setValue(true);
		}
		else
		{
			gMenuHolder->findControl(userdata["control"].asString())->setValue(false);
		}
		return true;
	}
};


/////////////
// PHANTOM //
/////////////

class LLAdvancedTogglePhantom: public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLAgent::togglePhantom();
		BOOL ph = LLAgent::getPhantom();
		LLChat chat;
		chat.mSourceType = CHAT_SOURCE_SYSTEM;
		chat.mText = llformat("%s%s","Phantom ",(ph ? "On" : "Off"));
		LLFloaterChat::addChat(chat);
		//gMenuHolder->findControl(userdata["control"].asString())->setValue(ph);
		return true;
	}

};

class LLAdvancedCheckPhantom: public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gMenuHolder->findControl(userdata["control"].asString())->setValue(LLAgent::getPhantom());
		return true;
	}
};


///////////////////
// ASSET BROWSER //
///////////////////

class LLAdvancedToggleAssetBrowser: public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		//open the floater
		LLFloaterAssetBrowser::show(0);
		
		bool vis = false;
		if(LLFloaterAssetBrowser::getInstance())
		{
			vis = (bool)LLFloaterAssetBrowser::getInstance()->getVisible();
		}
		return true;
	}
};

class LLAdvancedCheckAssetBrowser: public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool vis = false;
		if(LLFloaterAssetBrowser::getInstance())
		{
			vis = (bool)LLFloaterAssetBrowser::getInstance()->getVisible();
		}
		gMenuHolder->findControl(userdata["control"].asString())->setValue(vis);
		return true;
	}
};


///////////////////////
// CLEAR GROUP CACHE //
///////////////////////


class LLAdvancedClearGroupCache : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLGroupMgr::debugClearAllGroups(NULL);
		return true;
	}
};




/////////////////
// RENDER TYPE //
/////////////////


U32 render_type_from_string(std::string render_type)
{
	if ("simple" == render_type)
	{
		return LLPipeline::RENDER_TYPE_SIMPLE;
	}
	else if ("alpha" == render_type)
	{
		return LLPipeline::RENDER_TYPE_ALPHA;
	}
	else if ("tree" == render_type)
	{
		return LLPipeline::RENDER_TYPE_TREE;
	}
	else if ("avatar" == render_type)
	{
		return LLPipeline::RENDER_TYPE_AVATAR;
	}
	else if ("terrain" == render_type)
	{
		return LLPipeline::RENDER_TYPE_TERRAIN;
	}
	else if ("sky" == render_type)
	{
		return LLPipeline::RENDER_TYPE_SKY;
	}
	else if ("water" == render_type)
	{
		return LLPipeline::RENDER_TYPE_WATER;
	}
	else if ("ground" == render_type)
	{
		return LLPipeline::RENDER_TYPE_GROUND;
	}
	else if ("volume" == render_type)
	{
		return LLPipeline::RENDER_TYPE_VOLUME;
	}
	else if ("grass" == render_type)
	{
		return LLPipeline::RENDER_TYPE_GRASS;
	}
	else if ("clouds" == render_type)
	{
		return LLPipeline::RENDER_TYPE_CLOUDS;
	}
	else if ("particles" == render_type)
	{
		return LLPipeline::RENDER_TYPE_PARTICLES;
	}
	else if ("bump" == render_type)
	{
		return LLPipeline::RENDER_TYPE_BUMP;
	}
	else
	{
		return 0;
	}
}


class LLAdvancedToggleRenderType : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		U32 render_type = render_type_from_string( userdata.asString() );
		if ( render_type != 0 )
		{
			LLPipeline::toggleRenderTypeControl( (void*)render_type );
		}
		return true;
	}
};


class LLAdvancedCheckRenderType : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		U32 render_type = render_type_from_string( userdata["data"].asString() );
		bool new_value = false;

		if ( render_type != 0 )
		{
			new_value = LLPipeline::hasRenderTypeControl( (void*)render_type );
		}
		
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



/////////////
// FEATURE //
/////////////


U32 feature_from_string(std::string feature)
{
	if ("ui" == feature)
	{
		return LLPipeline::RENDER_DEBUG_FEATURE_UI;
	}
	else if ("selected" == feature)
	{
		return LLPipeline::RENDER_DEBUG_FEATURE_SELECTED;
	}
	else if ("highlighted" == feature)
	{
		return LLPipeline::RENDER_DEBUG_FEATURE_HIGHLIGHTED;
	}
	else if ("dynamic textures" == feature)
	{
		return LLPipeline::RENDER_DEBUG_FEATURE_DYNAMIC_TEXTURES;
	}
	else if ("foot shadows" == feature)
	{
		return LLPipeline::RENDER_DEBUG_FEATURE_FOOT_SHADOWS;
	}
	else if ("fog" == feature)
	{
		return LLPipeline::RENDER_DEBUG_FEATURE_FOG;
	}
	else if ("fr info" == feature)
	{
		return LLPipeline::RENDER_DEBUG_FEATURE_FR_INFO;
	}
	else if ("flexible" == feature)
	{
		return LLPipeline::RENDER_DEBUG_FEATURE_FLEXIBLE;
	}
	else
	{
		return 0;
	}
};


class LLAdvancedToggleFeature : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		U32 feature = feature_from_string( userdata.asString() );

		if ( feature != 0 )
		{
			LLPipeline::toggleRenderDebugFeature( (void*)feature );
		}

		return true;
	}
};


class LLAdvancedCheckFeature : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		U32 feature = feature_from_string( userdata["data"].asString() );
		bool new_value = false;

		if ( feature != 0 )
		{
			new_value = LLPipeline::toggleRenderDebugFeatureControl( (void*)feature );
		}
		
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



//////////////////
// INFO DISPLAY //
//////////////////


U32 info_display_from_string(std::string info_display)
{
	if ("verify" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_VERIFY;
	}
	else if ("bboxes" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_BBOXES;
	}
	else if ("points" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_POINTS;
	}
	else if ("octree" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_OCTREE;
	}
	else if ("occlusion" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_OCCLUSION;
	}
	else if ("render batches" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_BATCH_SIZE;
	}
	else if ("texture anim" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_TEXTURE_ANIM;
	}
	else if ("texture priority" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_TEXTURE_PRIORITY;
	}
	else if ("shame" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_SHAME;
	}
	else if ("texture area" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_TEXTURE_AREA;
	}
	else if ("face area" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_FACE_AREA;
	}
	else if ("picking" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_PICKING;
	}
	else if ("lights" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_LIGHTS;
	}
	else if ("particles" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_PARTICLES;
	}
	else if ("composition" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_COMPOSITION;
	}
	else if ("glow" == info_display)
	{
		return LLPipeline::RENDER_DEBUG_GLOW;
	}
	else
	{
		return 0;
	}
};


class LLAdvancedToggleInfoDisplay : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		U32 info_display = info_display_from_string( userdata.asString() );

		if ( info_display != 0 )
		{
			LLPipeline::toggleRenderDebug( (void*)info_display );
		}

		return true;
	}
};


class LLAdvancedCheckInfoDisplay : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		U32 info_display = info_display_from_string( userdata["data"].asString() );
		bool new_value = false;

		if ( info_display != 0 )
		{
			new_value = LLPipeline::toggleRenderDebugControl( (void*)info_display );
		}
		
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



///////////////////
// SELECT BUFFER //
///////////////////


class LLAdvancedToggleSelectBuffer : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gDebugSelect = !(gDebugSelect);
		return true;
	}
};

class LLAdvancedCheckSelectBuffer : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gDebugSelect;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



/////////////////////////
// RANDOMIZE FRAMERATE //
/////////////////////////


class LLAdvancedToggleRandomizeFramerate : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gRandomizeFramerate = !(gRandomizeFramerate);
		return true;
	}
};

class LLAdvancedCheckRandomizeFramerate : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gRandomizeFramerate;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



/////////////////////////
// PERIODIC SLOW FRAME //
/////////////////////////


class LLAdvancedTogglePeriodicSlowFrame : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gPeriodicSlowFrame = !(gPeriodicSlowFrame);
		return true;
	}
};

class LLAdvancedCheckPeriodicSlowFrame : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gPeriodicSlowFrame;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////
// FRAME TEST //
////////////////


class LLAdvancedToggleFrameTest : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLPipeline::sRenderFrameTest = !(LLPipeline::sRenderFrameTest);
		return true;
	}
};

class LLAdvancedCheckFrameTest : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLPipeline::sRenderFrameTest;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



///////////////////////////
// HIDE SELECTED OBJECTS //
///////////////////////////


class LLAdvancedToggleHideSelectedObjects : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gHideSelectedObjects = !(gHideSelectedObjects);
		return true;
	}
};

class LLAdvancedCheckHideSelectedObjects : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gHideSelectedObjects;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



///////////////////////////
// SELECTED TEXTURE INFO //
///////////////////////////


class LLAdvancedSelectedTextureInfo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_selected_texture_info(NULL);
		return true;
	}
};



//////////////////////
// TOGGLE WIREFRAME //
//////////////////////


class LLAdvancedToggleWireframe : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gUseWireframe = !(gUseWireframe);
		return true;
	}
};

class LLAdvancedCheckWireframe : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gUseWireframe;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



//////////////////////
// DISABLE TEXTURES //
//////////////////////


class LLAdvancedToggleDisableTextures : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		menu_toggle_variable((void*)&LLViewerImage::sDontLoadVolumeTextures);
		return true;
	}
};

class LLAdvancedCheckDisableTextures : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = menu_check_variable((void*)&LLViewerImage::sDontLoadVolumeTextures);
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////////////////////////////
// RENDER ATTACHED LIGHTS / PARTICLES //
////////////////////////////////////////


class LLToggleRenderAttachedLights : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
    menu_toggle_control((void *)"RenderAttachedLights");
    LLPipeline::sRenderAttachedLights = gSavedSettings.getBOOL("RenderAttachedLights");
		return true;
	}
};

class LLToggleRenderAttachedParticles : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
    menu_toggle_control((void *)"RenderAttachedParticles");
    LLPipeline::sRenderAttachedParticles = gSavedSettings.getBOOL("RenderAttachedParticles");
		return true;
	}
};



//////////////////////////
// DUMP SCRIPTED CAMERA //
//////////////////////////


class LLAdvancedDumpScriptedCamera : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_dump_followcam(NULL);
		return true;
	}
};



//////////////////////////////
// DUMP REGION OBJECT CACHE //
//////////////////////////////


class LLAdvancedDumpRegionObjectCache : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_dump_region_object_cache(NULL);
		return true;
	}
};



////////////////
// SLURL TEST //
////////////////


class LLAdvancedSLURLTest : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_slurl_test(NULL);
		return true;
	}
};



////////////////////////
// TOGGLE EDITABLE UI //
////////////////////////


class LLAdvancedToggleEditableUI : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		edit_ui(NULL);
		return true;
	}
};

// *TODO: Add corresponding "Check" for EditableUI, so it can
// become a menu_item_check. Need to add check_edit_ui(void*)
// or functional equivalent to do that.



/////////////////////
// DUMP SELECT MGR //
/////////////////////


class LLAdvancedDumpSelectMgr : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		dump_select_mgr(NULL);
		return true;
	}
};



////////////////////
// DUMP INVENTORY //
////////////////////


class LLAdvancedDumpInventory : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		dump_inventory(NULL);
		return true;
	}
};



///////////////////////
// DUMP FOCUS HOLDER //
///////////////////////


class LLAdvancedDumpFocusHolder : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_dump_focus(NULL);
		return true;
	}
};



////////////////////////////////
// PRINT SELECTED OBJECT INFO //
////////////////////////////////


class LLAdvancedPrintSelectedObjectInfo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		print_object_info(NULL);
		return true;
	}
};



//////////////////////
// PRINT AGENT INFO //
//////////////////////


class LLAdvancedPrintAgentInfo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		print_agent_nvpairs(NULL);
		return true;
	}
};



////////////////////////////////
// PRINT TEXTURE MEMORY STATS //
////////////////////////////////


class LLAdvancedPrintTextureMemoryStats : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		output_statistics(NULL);
		return true;
	}
};



//////////////////////
// DEBUG SELECT MGR //
//////////////////////


class LLAdvancedToggleDebugSelectMgr : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gDebugSelectMgr = !(gDebugSelectMgr);
		return true;
	}
};

class LLAdvancedCheckDebugSelectMgr : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gDebugSelectMgr;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



//////////////////
// DEBUG CLICKS //
//////////////////


class LLAdvancedToggleDebugClicks : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gDebugClicks = !(gDebugClicks);
		return true;
	}
};

class LLAdvancedCheckDebugClicks : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gDebugClicks;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



/////////////////
// DEBUG VIEWS //
/////////////////


class LLAdvancedToggleDebugViews : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLView::sDebugRects = !(LLView::sDebugRects);
		return true;
	}
};

class LLAdvancedCheckDebugViews : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLView::sDebugRects;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



///////////////////////
// XUI NAME TOOLTIPS //
///////////////////////


class LLAdvancedToggleXUINameTooltips : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		toggle_show_xui_names(NULL);
		return true;
	}
};

class LLAdvancedCheckXUINameTooltips : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = check_show_xui_names(NULL);
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////////////
// DEBUG MOUSE EVENTS //
////////////////////////


class LLAdvancedToggleDebugMouseEvents : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLView::sDebugMouseHandling = !(LLView::sDebugMouseHandling);
		return true;
	}
};

class LLAdvancedCheckDebugMouseEvents : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLView::sDebugMouseHandling;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////
// DEBUG KEYS //
////////////////


class LLAdvancedToggleDebugKeys : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLView::sDebugKeys = !(LLView::sDebugKeys);
		return true;
	}
};

class LLAdvancedCheckDebugKeys : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLView::sDebugKeys;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



///////////////////////
// DEBUG WINDOW PROC //
///////////////////////


class LLAdvancedToggleDebugWindowProc : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gDebugWindowProc = !(gDebugWindowProc);
		return true;
	}
};

class LLAdvancedCheckDebugWindowProc : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gDebugWindowProc;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////////////////
// DEBUG TEXT EDITOR TIPS //
////////////////////////////


class LLAdvancedToggleDebugTextEditorTips : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gDebugTextEditorTips = !(gDebugTextEditorTips);
		return true;
	}
};

class LLAdvancedCheckDebugTextEditorTips : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gDebugTextEditorTips;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



///////////////////////
// SHOW FLOATER TEST //
///////////////////////


class LLAdvancedShowFloaterTest : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLFloaterTest::show(NULL);
		return true;
	}
};



/////////////////////////
// EXPORT MENUS TO XML //
/////////////////////////


class LLAdvancedExportMenusToXML : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_export_menus_to_xml(NULL);
		return true;
	}
};



/////////////
// EDIT UI //
/////////////


class LLAdvancedEditUI : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLFloaterEditUI::show(NULL);
		return true;
	}
};



//////////////////////
// LOAD UI FROM XML //
//////////////////////


class LLAdvancedLoadUIFromXML : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_load_from_xml(NULL);
		return true;
	}
};



////////////////////
// SAVE UI TO XML //
////////////////////


class LLAdvancedSaveUIToXML : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_save_to_xml(NULL);
		return true;
	}
};



///////////////
// XUI NAMES //
///////////////


class LLAdvancedToggleXUINames : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		toggle_show_xui_names(NULL);
		return true;
	}
};

class LLAdvancedCheckXUINames : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = check_show_xui_names(NULL);
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////////////
// GRAB BAKED TEXTURE //
////////////////////////


class LLAdvancedGrabBakedTexture : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string texture_type = userdata.asString();
		if ("eyes" == texture_type)
		{
			handle_grab_texture( (void*)LLVOAvatar::TEX_EYES_BAKED );
		}
		else if ("head" == texture_type)
		{
			handle_grab_texture( (void*)LLVOAvatar::TEX_HEAD_BAKED );
		}
		else if ("upper" == texture_type)
		{
			handle_grab_texture( (void*)LLVOAvatar::TEX_UPPER_BAKED );
		}
		else if ("lower" == texture_type)
		{
			handle_grab_texture( (void*)LLVOAvatar::TEX_SKIRT_BAKED );
		}
		else if ("skirt" == texture_type)
		{
			handle_grab_texture( (void*)LLVOAvatar::TEX_SKIRT_BAKED );
		}

		return true;
	}
};

class LLAdvancedEnableGrabBakedTexture : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string texture_type = userdata["data"].asString();
		bool new_value = false;

		if ("iris" == texture_type)
		{
			new_value = enable_grab_texture( (void*)LLVOAvatar::TEX_EYES_BAKED );
		}
		else if ("head" == texture_type)
		{
			new_value = enable_grab_texture( (void*)LLVOAvatar::TEX_HEAD_BAKED );
		}
		else if ("upper" == texture_type)
		{
			new_value = enable_grab_texture( (void*)LLVOAvatar::TEX_UPPER_BAKED );
		}
		else if ("lower" == texture_type)
		{
			new_value = enable_grab_texture( (void*)LLVOAvatar::TEX_LOWER_BAKED );
		}
		else if ("skirt" == texture_type)
		{
			new_value = enable_grab_texture( (void*)LLVOAvatar::TEX_SKIRT_BAKED );
		}
		
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



//////////////////////
// ALLOW IDLE / AFK //
//////////////////////


class LLAdvancedToggleAllowIdleAFK : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gAllowIdleAFK = !(gAllowIdleAFK);
		return true;
	}
};

class LLAdvancedCheckAllowIdleAFK : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gAllowIdleAFK;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



///////////////////////
// APPEARANCE TO XML //
///////////////////////


class LLAdvancedAppearanceToXML : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar::dumpArchetypeXML(NULL);
		return true;
	}
};



///////////////////////////////
// TOGGLE CHARACTER GEOMETRY //
///////////////////////////////


class LLAdvancedToggleCharacterGeometry : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_god_request_avatar_geometry(NULL);
		return true;
	}
};

class LLAdvancedEnableCharacterGeometry : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		enable_god_customer_service(NULL);
		return true;
	}
};



/////////////////////////////
// TEST MALE / TEST FEMALE //
/////////////////////////////


class LLAdvancedTestMale : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_test_male(NULL);
		return true;
	}
};


class LLAdvancedTestFemale : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_test_female(NULL);
		return true;
	}
};



///////////////
// TOGGLE PG //
///////////////


class LLAdvancedTogglePG : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_toggle_pg(NULL);
		return true;
	}
};



/////////////////////////
// ALLOW SELECT AVATAR //
/////////////////////////


class LLAdvancedToggleAllowSelectAvatar : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gAllowSelectAvatar = !(gAllowSelectAvatar);
		return true;
	}
};

class LLAdvancedCheckAllowSelectAvatar : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gAllowSelectAvatar;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////////////////
// ALLOW TAP-TAP-HOLD RUN //
////////////////////////////


class LLAdvancedToggleAllowTapTapHoldRun : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gAllowTapTapHoldRun = !(gAllowTapTapHoldRun);
		return true;
	}
};

class LLAdvancedCheckAllowTapTapHoldRun : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gAllowTapTapHoldRun;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



/////////////////////////////
// FORCE PARAMS TO DEFAULT //
/////////////////////////////


class LLAdvancedForceParamsToDefault : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLAgent::clearVisualParams(NULL);
		return true;
	}
};



//////////////////////////
// RELOAD VERTEX SHADER //
//////////////////////////


class LLAdvancedReloadVertexShader : public view_listener_t
{
  bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
  {
    reload_vertex_shader(NULL);
    return true;
  }
};



////////////////////
// ANIMATION INFO //
////////////////////


class LLAdvancedToggleAnimationInfo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar::sShowAnimationDebug = !(LLVOAvatar::sShowAnimationDebug);
		return true;
	}
};

class LLAdvancedCheckAnimationInfo : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLVOAvatar::sShowAnimationDebug;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////////////////
// SLOW MOTION ANIMATIONS //
////////////////////////////


class LLAdvancedToggleSlowMotionAnimations : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		slow_mo_animations(NULL);
		return true;
	}
};

// *TODO: Add a corresponding "Check" event for SlowMotionAnimations,
// so that it can become a menu_item_check with the "X" indicator.
// See indra/newview/skins/xui/en_us/menu_viewer.xml



//////////////////
// SHOW LOOK AT //
//////////////////


class LLAdvancedToggleShowLookAt : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLHUDEffectLookAt::sDebugLookAt = !(LLHUDEffectLookAt::sDebugLookAt);
		return true;
	}
};

class LLAdvancedCheckShowLookAt : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLHUDEffectLookAt::sDebugLookAt;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



///////////////////
// SHOW POINT AT //
///////////////////


class LLAdvancedToggleShowPointAt : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLHUDEffectPointAt::sDebugPointAt = !(LLHUDEffectPointAt::sDebugPointAt);
		return true;
	}
};

class LLAdvancedCheckShowPointAt : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLHUDEffectPointAt::sDebugPointAt;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



/////////////////////////
// DEBUG JOINT UPDATES //
/////////////////////////


class LLAdvancedToggleDebugJointUpdates : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar::sJointDebug = !(LLVOAvatar::sJointDebug);
		return true;
	}
};

class LLAdvancedCheckDebugJointUpdates : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLVOAvatar::sJointDebug;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



/////////////////
// DISABLE LOD //
/////////////////


class LLAdvancedToggleDisableLOD : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerJoint::sDisableLOD = !(LLViewerJoint::sDisableLOD);
		return true;
	}
};

class LLAdvancedCheckDisableLOD : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLViewerJoint::sDisableLOD;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



/////////////////////////
// DEBUG CHARACTER VIS //
/////////////////////////


class LLAdvancedToggleDebugCharacterVis : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar::sDebugInvisible = !(LLVOAvatar::sDebugInvisible);
		return true;
	}
};

class LLAdvancedCheckDebugCharacterVis : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLVOAvatar::sDebugInvisible;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



//////////////////////////
// SHOW COLLISION PLANE //
//////////////////////////

/***************************
 *
 *  Disabled. See DEV-14477
 *
 ***************************

class LLAdvancedToggleShowCollisionPlane : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar::sShowFootPlane = !(LLVOAvatar::sShowFootPlane);
		return true;
	}
};

class LLAdvancedCheckShowCollisionPlane : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLVOAvatar::sShowFootPlane;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};

***************************/


/////////////////////////////
// SHOW COLLISION SKELETON //
/////////////////////////////


class LLAdvancedToggleShowCollisionSkeleton : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar::sShowCollisionVolumes = !(LLVOAvatar::sShowCollisionVolumes);
		return true;
	}
};

class LLAdvancedCheckShowCollisionSkeleton : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLVOAvatar::sShowCollisionVolumes;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



//////////////////////////
// DISPLAY AGENT TARGET //
//////////////////////////


class LLAdvancedToggleDisplayAgentTarget : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLAgent::sDebugDisplayTarget = !(LLAgent::sDebugDisplayTarget);
		return true;
	}
};

class LLAdvancedCheckDisplayAgentTarget : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLAgent::sDebugDisplayTarget;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



///////////////////////////
// DEBUG AVATAR ROTATION //
///////////////////////////


class LLAdvancedToggleDebugAvatarRotation : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gDebugAvatarRotation = !(gDebugAvatarRotation);
		return true;
	}
};

class LLAdvancedCheckDebugAvatarRotation : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gDebugAvatarRotation;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



//////////////////////
// DUMP ATTACHMENTS //
//////////////////////


class LLAdvancedDumpAttachments : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_dump_attachments(NULL);
		return true;
	}
};



/////////////////////
// REBAKE TEXTURES //
/////////////////////


class LLAdvancedRebakeTextures : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_rebake_textures(NULL);
		return true;
	}
};



///////////////////////////
// DEBUG AVATAR TEXTURES //
///////////////////////////


class LLAdvancedDebugAvatarTextures : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_debug_avatar_textures(NULL);
		return true;
	}
};



////////////////////////////////
// DUMP AVATAR LOCAL TEXTURES //
////////////////////////////////


class LLAdvancedDumpAvatarLocalTextures : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_dump_avatar_local_textures(NULL);
		return true;
	}
};



/////////////////
// MESSAGE LOG //
/////////////////


class LLAdvancedEnableMessageLog : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_viewer_enable_message_log(NULL);
		return true;
	}
};

class LLAdvancedDisableMessageLog : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_viewer_disable_message_log(NULL);
		return true;
	}
};



/////////////////
// DROP PACKET //
/////////////////


class LLAdvancedDropPacket : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		drop_packet(NULL);
		return true;
	}
};



/////////////////////////
// FRAME STATS LOGGING //
/////////////////////////


class LLAdvancedFrameStatsLogging : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string command = userdata.asString();
		if ("start logging" == command)
		{
			LLFrameStats::startLogging(NULL);
		}
		else if ("stop logging" == command)
		{
			LLFrameStats::stopLogging(NULL);
		}
		else if ("timed logging 10" == command)
		{
			LLFrameStats::timedLogging10(NULL);
		}
		else if ("timed logging 30" == command)
		{
			LLFrameStats::timedLogging30(NULL);
		}
		else if ("timed logging 60" == command)
		{
			LLFrameStats::timedLogging60(NULL);
		}

		return true;
	}		
};



/////////////////
// AGENT PILOT //
/////////////////


class LLAdvancedAgentPilot : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		std::string command = userdata.asString();
		if ("start playback" == command)
		{
			LLAgentPilot::startPlayback(NULL);
		}
		else if ("stop playback" == command)
		{
			LLAgentPilot::stopPlayback(NULL);
		}
		else if ("start record" == command)
		{
			LLAgentPilot::startRecord(NULL);
		}
		else if ("stop record" == command)
		{
			LLAgentPilot::saveRecord(NULL);
		}

		return true;
	}		
};



//////////////////////
// AGENT PILOT LOOP //
//////////////////////


class LLAdvancedToggleAgentPilotLoop : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLAgentPilot::sLoop = !(LLAgentPilot::sLoop);
		return true;
	}
};

class LLAdvancedCheckAgentPilotLoop : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = LLAgentPilot::sLoop;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



/////////////////////////
// SHOW OBJECT UPDATES //
/////////////////////////


class LLAdvancedToggleShowObjectUpdates : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gShowObjectUpdates = !(gShowObjectUpdates);
		return true;
	}
};

class LLAdvancedCheckShowObjectUpdates : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = gShowObjectUpdates;
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////////
// COMPRESS IMAGE //
////////////////////


class LLAdvancedCompressImage : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_compress_image(NULL);
		return true;
	}
};



//////////////////////
// CLOTHING FLOATER //
//////////////////////


class LLAdvancedToggleClothingFloater : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_clothing(NULL);
		return true;
	}
};

// There is no LLAdvancedCheckClothingFloater.



/////////////////////////
// SHOW DEBUG SETTINGS //
/////////////////////////


class LLAdvancedShowDebugSettings : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLFloaterSettingsDebug::show(NULL);
		return true;
	}
};



////////////////////////
// VIEW ADMIN OPTIONS //
////////////////////////


class LLAdvancedToggleViewAdminOptions : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_admin_override_toggle(NULL);
		return true;
	}
};

class LLAdvancedCheckViewAdminOptions : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = check_admin_override(NULL);
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



//////////////////
// ADMIN STATUS //
//////////////////


class LLAdvancedRequestAdminStatus : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_god_mode(NULL);
		return true;
	}
};

class LLAdvancedLeaveAdminStatus : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		handle_leave_god_mode(NULL);
		return true;
	}
};

class LLAvatarReportAbuse : public view_listener_t
{
		bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
		{
			LLVOAvatar* avatar = find_avatar_from_object( LLSelectMgr::getInstance()->getSelection()->getPrimaryObject() );
			if(avatar)
			{
				LLFloaterReporter::showFromObject(avatar->getID());
			}
			return true;
		}
};



///////////////
// RLVa Main //
///////////////


class RLVaMainToggle : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		rlvToggleEnabled(NULL);
		return true;
	}
};

class RLVaMainCheck : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		bool new_value = rlvGetEnabled(NULL);
		std::string control_name = userdata["control"].asString();
		gMenuHolder->findControl(control_name)->setValue(new_value);
		return true;
	}
};



////////////////////
// RLVa BEHAVIORS //
////////////////////


class RLVaBehaviorsShow : public view_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		RlvFloaterBehaviour::show(NULL);
		return true;
	}
};



static void addMenu(view_listener_t *menu, const char *name)
{
	sMenus.push_back(menu);
	menu->registerListener(gMenuHolder, name);
}

void initialize_menus()
{
	// File menu
	init_menu_file();

	// Edit menu
	addMenu(new LLEditUndo(), "Edit.Undo");
	addMenu(new LLEditRedo(), "Edit.Redo");
	addMenu(new LLEditCut(), "Edit.Cut");
	addMenu(new LLEditCopy(), "Edit.Copy");
	addMenu(new LLEditPaste(), "Edit.Paste");
	addMenu(new LLEditDelete(), "Edit.Delete");
	addMenu(new LLEditSearch(), "Edit.Search");
	addMenu(new LLEditSelectAll(), "Edit.SelectAll");
	addMenu(new LLEditDeselect(), "Edit.Deselect");
	addMenu(new LLEditDuplicate(), "Edit.Duplicate");
	addMenu(new LLEditTakeOff(), "Edit.TakeOff");

	addMenu(new LLEditEnableUndo(), "Edit.EnableUndo");
	addMenu(new LLEditEnableRedo(), "Edit.EnableRedo");
	addMenu(new LLEditEnableCut(), "Edit.EnableCut");
	addMenu(new LLEditEnableCopy(), "Edit.EnableCopy");
	addMenu(new LLEditEnablePaste(), "Edit.EnablePaste");
	addMenu(new LLEditEnableDelete(), "Edit.EnableDelete");
	addMenu(new LLEditEnableSelectAll(), "Edit.EnableSelectAll");
	addMenu(new LLEditEnableDeselect(), "Edit.EnableDeselect");
	addMenu(new LLEditEnableDuplicate(), "Edit.EnableDuplicate");
	addMenu(new LLEditEnableTakeOff(), "Edit.EnableTakeOff");
	addMenu(new LLEditEnableCustomizeAvatar(), "Edit.EnableCustomizeAvatar");
	addMenu(new LLAdvancedRebakeTextures(), "Advanced.RebakeTextures");

	// View menu
	addMenu(new LLViewMouselook(), "View.Mouselook");
	addMenu(new LLViewBuildMode(), "View.BuildMode");
	addMenu(new LLViewJoystickFlycam(), "View.JoystickFlycam");
	addMenu(new LLViewCommunicate(), "View.Communicate");
	addMenu(new LLViewResetView(), "View.ResetView");
	addMenu(new LLViewLookAtLastChatter(), "View.LookAtLastChatter");
	addMenu(new LLViewShowHoverTips(), "View.ShowHoverTips");
	addMenu(new LLViewHighlightTransparent(), "View.HighlightTransparent");
	addMenu(new LLViewToggleRenderType(), "View.ToggleRenderType");
	addMenu(new LLViewShowHUDAttachments(), "View.ShowHUDAttachments");
	addMenu(new LLViewZoomOut(), "View.ZoomOut");
	addMenu(new LLViewZoomIn(), "View.ZoomIn");
	addMenu(new LLViewZoomDefault(), "View.ZoomDefault");
	addMenu(new LLViewFullscreen(), "View.Fullscreen");
	addMenu(new LLViewToggleAdvanced(), "View.ToggleAdvanced");


	addMenu(new LLViewEnableMouselook(), "View.EnableMouselook");
	addMenu(new LLViewEnableLastChatter(), "View.EnableLastChatter");

	addMenu(new LLViewCheckBuildMode(), "View.CheckBuildMode");
	addMenu(new LLViewCheckJoystickFlycam(), "View.CheckJoystickFlycam");
	addMenu(new LLViewCheckShowHoverTips(), "View.CheckShowHoverTips");
	addMenu(new LLViewCheckHighlightTransparent(), "View.CheckHighlightTransparent");
	addMenu(new LLViewCheckRenderType(), "View.CheckRenderType");
	addMenu(new LLViewCheckHUDAttachments(), "View.CheckHUDAttachments");
	addMenu(new LLViewCheckAdvanced(), "View.CheckAdvanced");

	// World menu
	addMenu(new LLWorldChat(), "World.Chat");
	addMenu(new LLWorldAlwaysRun(), "World.AlwaysRun");
	addMenu(new LLWorldFly(), "World.Fly");
	addMenu(new LLWorldEnableFly(), "World.EnableFly");
	addMenu(new LLWorldCreateLandmark(), "World.CreateLandmark");
	addMenu(new LLWorldSetHomeLocation(), "World.SetHomeLocation");
	addMenu(new LLWorldTeleportHome(), "World.TeleportHome");
	addMenu(new LLWorldSetAway(), "World.SetAway");
	addMenu(new LLWorldSetBusy(), "World.SetBusy");

	addMenu(new LLWorldEnableCreateLandmark(), "World.EnableCreateLandmark");
	addMenu(new LLWorldEnableSetHomeLocation(), "World.EnableSetHomeLocation");
	addMenu(new LLWorldEnableTeleportHome(), "World.EnableTeleportHome");
	addMenu(new LLWorldEnableBuyLand(), "World.EnableBuyLand");

	addMenu(new LLWorldCheckAlwaysRun(), "World.CheckAlwaysRun");
	
	(new LLWorldEnvSettings())->registerListener(gMenuHolder, "World.EnvSettings");
	(new LLWorldWaterSettings())->registerListener(gMenuHolder, "World.WaterSettings");
	(new LLWorldPostProcess())->registerListener(gMenuHolder, "World.PostProcess");
	(new LLWorldDayCycle())->registerListener(gMenuHolder, "World.DayCycle");

	// Tools menu
	addMenu(new LLToolsSelectTool(), "Tools.SelectTool");
	addMenu(new LLToolsSelectOnlyMyObjects(), "Tools.SelectOnlyMyObjects");
	addMenu(new LLToolsSelectOnlyMovableObjects(), "Tools.SelectOnlyMovableObjects");
	addMenu(new LLToolsSelectOnlyCopyableObjects(), "Tools.SelectOnlyCopyableObjects");
	addMenu(new LLToolsSelectBySurrounding(), "Tools.SelectBySurrounding");
	addMenu(new LLToolsShowSelectionHighlights(), "Tools.ShowSelectionHighlights");
	addMenu(new LLToolsShowHiddenSelection(), "Tools.ShowHiddenSelection");
	addMenu(new LLToolsShowSelectionLightRadius(), "Tools.ShowSelectionLightRadius");
	addMenu(new LLToolsEditLinkedParts(), "Tools.EditLinkedParts");
	addMenu(new LLToolsSnapObjectXY(), "Tools.SnapObjectXY");
	addMenu(new LLToolsUseSelectionForGrid(), "Tools.UseSelectionForGrid");
	addMenu(new LLToolsLink(), "Tools.Link");
	addMenu(new LLToolsUnlink(), "Tools.Unlink");
	addMenu(new LLToolsStopAllAnimations(), "Tools.StopAllAnimations");
	addMenu(new LLToolsReleaseKeys(), "Tools.ReleaseKeys");
	addMenu(new LLToolsEnableReleaseKeys(), "Tools.EnableReleaseKeys");
	addMenu(new LLToolsLookAtSelection(), "Tools.LookAtSelection");
	addMenu(new LLToolsTake(), "Tools.Take");
	addMenu(new LLToolsTakeCopy(), "Tools.TakeCopy");
	addMenu(new LLToolsSaveToInventory(), "Tools.SaveToInventory");
	addMenu(new LLToolsSaveToObjectInventory(), "Tools.SaveToObjectInventory");
	addMenu(new LLToolsSelectedScriptAction(), "Tools.SelectedScriptAction");
	addMenu(new LLToolsSetBulkPerms(), "Tools.SetBulkPerms");

	addMenu(new LLToolsEnableToolNotPie(), "Tools.EnableToolNotPie");
	addMenu(new LLToolsEnableLink(), "Tools.EnableLink");
	addMenu(new LLToolsEnableUnlink(), "Tools.EnableUnlink");
	addMenu(new LLToolsEnableTake(), "Tools.EnableTake");
	addMenu(new LLToolsEnableTakeCopy(), "Tools.EnableTakeCopy");
	addMenu(new LLToolsEnableSaveToInventory(), "Tools.SaveToInventory");
	addMenu(new LLToolsEnableSaveToObjectInventory(), "Tools.SaveToObjectInventory");

	/*addMenu(new LLToolsVisibleBuyObject(), "Tools.VisibleBuyObject");
	addMenu(new LLToolsVisibleTakeObject(), "Tools.VisibleTakeObject");*/

	// Help menu
	// most items use the ShowFloater method

	// Self pie menu
	addMenu(new LLSelfStandUp(), "Self.StandUp");
	addMenu(new LLSelfRemoveAllAttachments(), "Self.RemoveAllAttachments");

	addMenu(new LLSelfEnableStandUp(), "Self.EnableStandUp");
	addMenu(new LLSelfEnableRemoveAllAttachments(), "Self.EnableRemoveAllAttachments");

	 // Avatar pie menu
	addMenu(new LLObjectMute(), "Avatar.Mute");
	addMenu(new LLAvatarAddFriend(), "Avatar.AddFriend");
	addMenu(new LLAvatarFreeze(), "Avatar.Freeze");
	addMenu(new LLAvatarDebug(), "Avatar.Debug");
	addMenu(new LLAvatarVisibleDebug(), "Avatar.VisibleDebug");
	addMenu(new LLAvatarEnableDebug(), "Avatar.EnableDebug");
	addMenu(new LLAvatarInviteToGroup(), "Avatar.InviteToGroup");
	addMenu(new LLAvatarGiveCard(), "Avatar.GiveCard");
	addMenu(new LLAvatarEject(), "Avatar.Eject");
	addMenu(new LLAvatarSendIM(), "Avatar.SendIM");
	addMenu(new LLAvatarReportAbuse(), "Avatar.ReportAbuse");
	
	addMenu(new LLObjectEnableMute(), "Avatar.EnableMute");
	addMenu(new LLAvatarEnableAddFriend(), "Avatar.EnableAddFriend");
	addMenu(new LLAvatarEnableFreezeEject(), "Avatar.EnableFreezeEject");

	// Object pie menu
	addMenu(new LLObjectOpen(), "Object.Open");
	addMenu(new LLObjectBuild(), "Object.Build");
	addMenu(new LLObjectTouch(), "Object.Touch");
	addMenu(new LLObjectSitOrStand(), "Object.SitOrStand");
	addMenu(new LLObjectDelete(), "Object.Delete");
	addMenu(new LLObjectAttachToAvatar(), "Object.AttachToAvatar");
	addMenu(new LLObjectReturn(), "Object.Return");
	addMenu(new LLObjectReportAbuse(), "Object.ReportAbuse");
	addMenu(new LLObjectMute(), "Object.Mute");
	addMenu(new LLObjectBuy(), "Object.Buy");
	addMenu(new LLObjectEdit(), "Object.Edit");
	addMenu(new LLObjectInspect(), "Object.Inspect");
	addMenu(new LLObjectCopyUUID(), "Object.CopyUUID");
	addMenu(new LLObjectExport(), "Object.Export");
	addMenu(new LLObjectImport(), "Object.Import");
	addMenu(new LLObjectImportUpload(), "Object.ImportUpload");

	addMenu(new LLObjectEnableOpen(), "Object.EnableOpen");
	addMenu(new LLObjectEnableTouch(), "Object.EnableTouch");
	addMenu(new LLObjectEnableSitOrStand(), "Object.EnableSitOrStand");
	addMenu(new LLObjectEnableDelete(), "Object.EnableDelete");
	addMenu(new LLObjectEnableWear(), "Object.EnableWear");
	addMenu(new LLObjectEnableReturn(), "Object.EnableReturn");
	addMenu(new LLObjectEnableReportAbuse(), "Object.EnableReportAbuse");
	addMenu(new LLObjectEnableMute(), "Object.EnableMute");
	addMenu(new LLObjectEnableBuy(), "Object.EnableBuy");
	addMenu(new LLObjectEnableCopyUUID(), "Object.EnableCopyUUID");
	addMenu(new LLObjectEnableExport(), "Object.EnableExport");
	addMenu(new LLObjectEnableImport(), "Object.EnableImport");

	/*addMenu(new LLObjectVisibleTouch(), "Object.VisibleTouch");
	addMenu(new LLObjectVisibleCustomTouch(), "Object.VisibleCustomTouch");
	addMenu(new LLObjectVisibleStandUp(), "Object.VisibleStandUp");
	addMenu(new LLObjectVisibleSitHere(), "Object.VisibleSitHere");
	addMenu(new LLObjectVisibleCustomSit(), "Object.VisibleCustomSit");*/

	// Attachment pie menu
	addMenu(new LLAttachmentDrop(), "Attachment.Drop");
	addMenu(new LLAttachmentDetach(), "Attachment.Detach");

	addMenu(new LLAttachmentEnableDrop(), "Attachment.EnableDrop");
	addMenu(new LLAttachmentEnableDetach(), "Attachment.EnableDetach");
	addMenu(new LLAttachmentEnableTouch(), "Attachment.EnableTouch");

	// Land pie menu
	addMenu(new LLLandBuild(), "Land.Build");
	addMenu(new LLLandSit(), "Land.Sit");
	addMenu(new LLWorldCreateLandmark(),"Land.NewLandmark");
	addMenu(new LLLandBuyPass(), "Land.BuyPass");
	addMenu(new LLLandEdit(), "Land.Edit");

	addMenu(new LLLandEnableBuyPass(), "Land.EnableBuyPass");

	// Generic actions
	addMenu(new LLShowFloater(), "ShowFloater");
	addMenu(new LLPromptShowURL(), "PromptShowURL");
	addMenu(new LLShowAgentProfile(), "ShowAgentProfile");
	addMenu(new LLShowAgentGroups(), "ShowAgentGroups");
	addMenu(new LLToggleControl(), "ToggleControl");

	addMenu(new LLGoToObject(), "GoToObject");
	addMenu(new LLPayObject(), "PayObject");

	addMenu(new LLEnablePayObject(), "EnablePayObject");
	addMenu(new LLEnableEdit(), "EnableEdit");

	addMenu(new LLFloaterVisible(), "FloaterVisible");
	addMenu(new LLSomethingSelected(), "SomethingSelected");
	addMenu(new LLSomethingSelectedNoHUD(), "SomethingSelectedNoHUD");
	addMenu(new LLEditableSelected(), "EditableSelected");
	addMenu(new LLEditableSelectedMono(), "EditableSelectedMono");


	// Advanced (top level menu)
	addMenu(new LLAdvancedToggleConsole(), "Advanced.ToggleConsole");
	addMenu(new LLAdvancedCheckConsole(), "Advanced.CheckConsole");
	addMenu(new LLAdvancedDumpInfoToConsole(), "Advanced.DumpInfoToConsole");
	addMenu(new LLAdvancedReloadSettingsOverrides(), "Advanced.ReloadSettingsOverrides");
	addMenu(new LLAdvancedToggleSit(), "Advanced.ToggleSit");
	addMenu(new LLAdvancedCheckSit(), "Emerald.CheckSit");
	addMenu(new LLAdvancedTogglePhantom(), "Advanced.TogglePhantom");
	addMenu(new LLAdvancedCheckPhantom(), "Advanced.CheckPhantom");
	addMenu(new LLAdvancedToggleAssetBrowser(),"Advanced.ToggleAssetBrowser");
	addMenu(new LLAdvancedCheckAssetBrowser(),"Advanced.CheckAssetBrowser");

	// Advanced > HUD Info
	addMenu(new LLAdvancedToggleHUDInfo(), "Advanced.ToggleHUDInfo");
	addMenu(new LLAdvancedCheckHUDInfo(), "Advanced.CheckHUDInfo");

	addMenu(new LLAdvancedClearGroupCache(), "Advanced.ClearGroupCache");

	// Advanced > Render > Types
	addMenu(new LLAdvancedToggleRenderType(), "Advanced.ToggleRenderType");
	addMenu(new LLAdvancedCheckRenderType(), "Advanced.CheckRenderType");

	// Advanced > Render > Features
	addMenu(new LLAdvancedToggleFeature(), "Advanced.ToggleFeature");
	addMenu(new LLAdvancedCheckFeature(), "Advanced.CheckFeature");

	// Advanced > Render > Info Displays
	addMenu(new LLAdvancedToggleInfoDisplay(), "Advanced.ToggleInfoDisplay");
	addMenu(new LLAdvancedCheckInfoDisplay(), "Advanced.CheckInfoDisplay");
	addMenu(new LLAdvancedToggleSelectBuffer(), "Advanced.ToggleSelectBuffer");
	addMenu(new LLAdvancedCheckSelectBuffer(), "Advanced.CheckSelectBuffer");
	addMenu(new LLAdvancedToggleRandomizeFramerate(), "Advanced.ToggleRandomizeFramerate");
	addMenu(new LLAdvancedCheckRandomizeFramerate(), "Advanced.CheckRandomizeFramerate");
	addMenu(new LLAdvancedTogglePeriodicSlowFrame(), "Advanced.TogglePeriodicSlowFrame");
	addMenu(new LLAdvancedCheckPeriodicSlowFrame(), "Advanced.CheckPeriodicSlowFrame");
	addMenu(new LLAdvancedToggleFrameTest(), "Advanced.ToggleFrameTest");
	addMenu(new LLAdvancedCheckFrameTest(), "Advanced.CheckFrameTest");
	addMenu(new LLAdvancedToggleHideSelectedObjects(), "Advanced.ToggleHideSelectedObjects");
	addMenu(new LLAdvancedCheckHideSelectedObjects(), "Advanced.CheckHideSelectedObjects");
	addMenu(new LLAdvancedSelectedTextureInfo(), "Advanced.SelectedTextureInfo");
	addMenu(new LLAdvancedToggleWireframe(), "Advanced.ToggleWireframe");
	addMenu(new LLAdvancedCheckWireframe(), "Advanced.CheckWireframe");
	addMenu(new LLAdvancedToggleDisableTextures(), "Advanced.ToggleDisableTextures");
	addMenu(new LLAdvancedCheckDisableTextures(), "Advanced.CheckDisableTextures");

	// Advanced > Render (top level menu)
	addMenu(new LLToggleRenderAttachedLights(), "ToggleRenderAttachedLights");
	addMenu(new LLToggleRenderAttachedParticles(), "ToggleRenderAttachedParticles");

	// Advanced > World
	addMenu(new LLAdvancedDumpScriptedCamera(), "Advanced.DumpScriptedCamera");
	addMenu(new LLAdvancedDumpRegionObjectCache(), "Advanced.DumpRegionObjectCache");

	// Advanced > UI
	addMenu(new LLAdvancedSLURLTest(), "Advanced.SLURLTest");
	addMenu(new LLAdvancedToggleEditableUI(), "Advanced.ToggleEditableUI");
	//addMenu(new LLAdvancedCheckEditableUI(), "Advanced.CheckEditableUI");
	addMenu(new LLAdvancedDumpSelectMgr(), "Advanced.DumpSelectMgr");
	addMenu(new LLAdvancedDumpInventory(), "Advanced.DumpInventory");
	addMenu(new LLAdvancedDumpFocusHolder(), "Advanced.DumpFocusHolder");
	addMenu(new LLAdvancedPrintSelectedObjectInfo(), "Advanced.PrintSelectedObjectInfo");
	addMenu(new LLAdvancedPrintAgentInfo(), "Advanced.PrintAgentInfo");
	addMenu(new LLAdvancedPrintTextureMemoryStats(), "Advanced.PrintTextureMemoryStats");
	addMenu(new LLAdvancedToggleDebugSelectMgr(), "Advanced.ToggleDebugSelectMgr");
	addMenu(new LLAdvancedCheckDebugSelectMgr(), "Advanced.CheckDebugSelectMgr");
	addMenu(new LLAdvancedToggleDebugClicks(), "Advanced.ToggleDebugClicks");
	addMenu(new LLAdvancedCheckDebugClicks(), "Advanced.CheckDebugClicks");
	addMenu(new LLAdvancedCheckDebugViews(), "Advanced.CheckDebugViews");
	addMenu(new LLAdvancedToggleDebugViews(), "Advanced.ToggleDebugViews");
	addMenu(new LLAdvancedToggleXUINameTooltips(), "Advanced.ToggleXUINameTooltips");
	addMenu(new LLAdvancedCheckXUINameTooltips(), "Advanced.CheckXUINameTooltips");
	addMenu(new LLAdvancedToggleDebugMouseEvents(), "Advanced.ToggleDebugMouseEvents");
	addMenu(new LLAdvancedCheckDebugMouseEvents(), "Advanced.CheckDebugMouseEvents");
	addMenu(new LLAdvancedToggleDebugKeys(), "Advanced.ToggleDebugKeys");
	addMenu(new LLAdvancedCheckDebugKeys(), "Advanced.CheckDebugKeys");
	addMenu(new LLAdvancedToggleDebugWindowProc(), "Advanced.ToggleDebugWindowProc");
	addMenu(new LLAdvancedCheckDebugWindowProc(), "Advanced.CheckDebugWindowProc");
	addMenu(new LLAdvancedToggleDebugTextEditorTips(), "Advanced.ToggleDebugTextEditorTips");
	addMenu(new LLAdvancedCheckDebugTextEditorTips(), "Advanced.CheckDebugTextEditorTips");

	// Advanced > XUI
	addMenu(new LLAdvancedShowFloaterTest(), "Advanced.ShowFloaterTest");
	addMenu(new LLAdvancedExportMenusToXML(), "Advanced.ExportMenusToXML");
	addMenu(new LLAdvancedEditUI(), "Advanced.EditUI");
	addMenu(new LLAdvancedLoadUIFromXML(), "Advanced.LoadUIFromXML");
	addMenu(new LLAdvancedSaveUIToXML(), "Advanced.SaveUIToXML");
	addMenu(new LLAdvancedToggleXUINames(), "Advanced.ToggleXUINames");
	addMenu(new LLAdvancedCheckXUINames(), "Advanced.CheckXUINames");

	// Advanced > Character > Grab Baked Texture
	addMenu(new LLAdvancedGrabBakedTexture(), "Advanced.GrabBakedTexture");
	addMenu(new LLAdvancedEnableGrabBakedTexture(), "Advanced.EnableGrabBakedTexture");

	// Advanced > Character > Character Tests
	addMenu(new LLAdvancedToggleAllowIdleAFK(), "Advanced.ToggleAllowIdleAFK");
	addMenu(new LLAdvancedCheckAllowIdleAFK(), "Advanced.CheckAllowIdleAFK");
	addMenu(new LLAdvancedAppearanceToXML(), "Advanced.AppearanceToXML");
	addMenu(new LLAdvancedToggleCharacterGeometry(), "Advanced.ToggleCharacterGeometry");
	addMenu(new LLAdvancedTestMale(), "Advanced.TestMale");
	addMenu(new LLAdvancedTestFemale(), "Advanced.TestFemale");
	addMenu(new LLAdvancedTogglePG(), "Advanced.TogglePG");
	addMenu(new LLAdvancedToggleAllowSelectAvatar(), "Advanced.ToggleAllowSelectAvatar");
	addMenu(new LLAdvancedCheckAllowSelectAvatar(), "Advanced.CheckAllowSelectAvatar");

	// Advanced > Character (toplevel)
	addMenu(new LLAdvancedToggleAllowTapTapHoldRun(), "Advanced.ToggleAllowTapTapHoldRun");
	addMenu(new LLAdvancedCheckAllowTapTapHoldRun(), "Advanced.CheckAllowTapTapHoldRun");
	addMenu(new LLAdvancedForceParamsToDefault(), "Advanced.ForceParamsToDefault");
	addMenu(new LLAdvancedReloadVertexShader(), "Advanced.ReloadVertexShader");
	addMenu(new LLAdvancedToggleAnimationInfo(), "Advanced.ToggleAnimationInfo");
	addMenu(new LLAdvancedCheckAnimationInfo(), "Advanced.CheckAnimationInfo");
	addMenu(new LLAdvancedToggleSlowMotionAnimations(), "Advanced.ToggleSlowMotionAnimations");
	//addMenu(new LLAdvancedCheckSlowMotionAnimations(), "Advanced.CheckSlowMotionAnimations");
	addMenu(new LLAdvancedToggleShowLookAt(), "Advanced.ToggleShowLookAt");
	addMenu(new LLAdvancedCheckShowLookAt(), "Advanced.CheckShowLookAt");
	addMenu(new LLAdvancedToggleShowPointAt(), "Advanced.ToggleShowPointAt");
	addMenu(new LLAdvancedCheckShowPointAt(), "Advanced.CheckShowPointAt");
	addMenu(new LLAdvancedToggleDebugJointUpdates(), "Advanced.ToggleDebugJointUpdates");
	addMenu(new LLAdvancedCheckDebugJointUpdates(), "Advanced.CheckDebugJointUpdates");
	addMenu(new LLAdvancedToggleDisableLOD(), "Advanced.ToggleDisableLOD");
	addMenu(new LLAdvancedCheckDisableLOD(), "Advanced.CheckDisableLOD");
	addMenu(new LLAdvancedToggleDebugCharacterVis(), "Advanced.ToggleDebugCharacterVis");
	addMenu(new LLAdvancedCheckDebugCharacterVis(), "Advanced.CheckDebugCharacterVis");
// 	addMenu(new LLAdvancedToggleShowCollisionPlane(), "Advanced.ToggleShowCollisionPlane");
// 	addMenu(new LLAdvancedCheckShowCollisionPlane(), "Advanced.CheckShowCollisionPlane");
	addMenu(new LLAdvancedToggleShowCollisionSkeleton(), "Advanced.ToggleShowCollisionSkeleton");
	addMenu(new LLAdvancedCheckShowCollisionSkeleton(), "Advanced.CheckShowCollisionSkeleton");
	addMenu(new LLAdvancedToggleDisplayAgentTarget(), "Advanced.ToggleDisplayAgentTarget");
	addMenu(new LLAdvancedCheckDisplayAgentTarget(), "Advanced.CheckDisplayAgentTarget");
	addMenu(new LLAdvancedToggleDebugAvatarRotation(), "Advanced.ToggleDebugAvatarRotation");
	addMenu(new LLAdvancedCheckDebugAvatarRotation(), "Advanced.CheckDebugAvatarRotation");
	addMenu(new LLAdvancedDumpAttachments(), "Advanced.DumpAttachments");
	addMenu(new LLAdvancedDebugAvatarTextures(), "Advanced.DebugAvatarTextures");
	addMenu(new LLAdvancedDumpAvatarLocalTextures(), "Advanced.DumpAvatarLocalTextures");

	// Advanced > Network
	addMenu(new LLAdvancedEnableMessageLog(), "Advanced.EnableMessageLog");
	addMenu(new LLAdvancedDisableMessageLog(), "Advanced.DisableMessageLog");
	addMenu(new LLAdvancedDropPacket(), "Advanced.DropPacket");

	// Advanced > Recorder
	addMenu(new LLAdvancedFrameStatsLogging(), "Advanced.FrameStatsLogging");
	addMenu(new LLAdvancedAgentPilot(), "Advanced.AgentPilot");
	addMenu(new LLAdvancedToggleAgentPilotLoop(), "Advanced.ToggleAgentPilotLoop");
	addMenu(new LLAdvancedCheckAgentPilotLoop(), "Advanced.CheckAgentPilotLoop");

	// Advanced (toplevel)
	addMenu(new LLAdvancedToggleShowObjectUpdates(), "Advanced.ToggleShowObjectUpdates");
	addMenu(new LLAdvancedCheckShowObjectUpdates(), "Advanced.CheckShowObjectUpdates");
	addMenu(new LLAdvancedCompressImage(), "Advanced.CompressImage");
	addMenu(new LLAdvancedToggleClothingFloater(), "Advanced.ToggleClothingFloater");
	addMenu(new LLAdvancedShowDebugSettings(), "Advanced.ShowDebugSettings");
	addMenu(new LLAdvancedToggleViewAdminOptions(), "Advanced.ToggleViewAdminOptions");
	addMenu(new LLAdvancedCheckViewAdminOptions(), "Advanced.CheckViewAdminOptions");
	addMenu(new LLAdvancedRequestAdminStatus(), "Advanced.RequestAdminStatus");
	addMenu(new LLAdvancedLeaveAdminStatus(), "Advanced.LeaveAdminStatus");


	// RLVa
	addMenu(new RLVaMainToggle(), "RLVa.Main.Toggle");
	addMenu(new RLVaMainCheck(), "RLVa.Main.Enabled");
	addMenu(new RLVaBehaviorsShow(), "RLVa.Behaviors.Show");

}
