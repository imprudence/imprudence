//
// C++ Interface: llfloateravatarlist
//
// Description: 
//
//
// Original author: Dale Glass <dale@daleglass.net>, (C) 2007
// Heavily modified by Henri Beauchamp 10/2009.
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "llfloater.h"
#include "llfloaterreporter.h"
#include "lluuid.h"
#include "lltimer.h"
#include "llscrolllistctrl.h"

#include <time.h>
#include <map>
#include <set>

class LLFloaterAvatarList;

/**
 * @brief This class is used to hold data about avatars.
 * We cache data about avatars to avoid repeating requests in this class.
 * Instances are kept in a map<LLAvatarListEntry>. We keep track of the
 * frame where the avatar was last seen.
 */
class LLAvatarListEntry {

public:

	/**
	 * @brief Initializes a list entry
	 * @param id Avatar's key
	 * @param name Avatar's name
	 * @param position Avatar's current position
	 */
	LLAvatarListEntry(const LLUUID& id = LLUUID::null, const std::string &name = "", const LLVector3d &position = LLVector3d::zero);

	/**
	 * Update world position.
	 * Affects age.
	 */	
	void setPosition(LLVector3d position, bool this_sim, bool drawn, bool chatrange, bool shoutrange);

	LLVector3d getPosition() { return mPosition; }

	/**
	 * @brief Returns the age of this entry in frames
	 *
	 * This is only used for determining whether the avatar is still around.
	 * @see getEntryAgeSeconds
	 */
	bool getAlive();

	/**
	 * @brief Returns the age of this entry in seconds
	 */
	F32 getEntryAgeSeconds();

	/**
	 * @brief Returns the name of the avatar
	 */
	std::string getName() { return mName; }

	/**
	 * @brief Sets the display name of the avatar
	 */
	void setDisplayName(std::string name) { mDisplayName = name; }

	/**
	 * @brief Returns the display name of the avatar
	 */
	std::string getDisplayName() { return mDisplayName; }

	/**
	 * @brief Returns the ID of the avatar
	 */
	LLUUID getID() { return mID; }

	/**
	 * @brief Sets the 'focus' status on this entry (camera focused on this avatar)
	 */
	void setFocus(BOOL value) { mFocused = value; }

	BOOL isFocused() { return mFocused; }

	BOOL isMarked() { return mMarked; }

	BOOL isDrawn() { return (mInDrawFrame != U32_MAX); }

	BOOL isInSim() { return (mInSimFrame != U32_MAX); }

	/**
	 * @brief Returns whether the item is dead and shouldn't appear in the list
	 * @returns TRUE if dead
	 */
	BOOL isDead();

	void toggleMark() { mMarked = !mMarked; }

private:
	friend class LLFloaterAvatarList;

	LLUUID mID;
	std::string mName;
	std::string mDisplayName;
	LLVector3d mPosition;
	LLVector3d mDrawPosition;
	BOOL mMarked;
	BOOL mFocused;

	/**
	 * @brief Timer to keep track of whether avatars are still there
	 */
	LLTimer mUpdateTimer;

	/**
	 * @brief Last frame when this avatar was updated
	 */
	U32 mFrame;
	//last frame when this avatar was in sim
	U32 mInSimFrame;
	//last frame when this avatar was in draw
	U32 mInDrawFrame;
	//last frame when this avatar was in shout range
	U32 mInShoutFrame;
	//last frame when this avatar was in chat range
	U32 mInChatFrame;
};


/**
 * @brief Avatar List
 * Implements an avatar scanner in the client.
 *
 * This is my first attempt to modify the SL source. This code is intended
 * to have a dual purpose: doing the task, and providing an example of how
 * to do it. For that reason, it's going to be commented as exhaustively
 * as possible.
 *
 * Since I'm very new to C++ any suggestions on coding, style, etc are very
 * welcome.
 */
class LLFloaterAvatarList : public LLFloater
{
	/**
	 * @brief Creates and initializes the LLFloaterAvatarList
	 * Here the interface is created, and callbacks are initialized.
	 */
private:
	LLFloaterAvatarList();
public:
	~LLFloaterAvatarList();

	/*virtual*/ void onClose(bool app_quitting);
	/*virtual*/ void onOpen();
	/*virtual*/ BOOL postBuild();
	/*virtual*/ void draw();

	/**
	 * @brief Toggles interface visibility
	 * There is only one instance of the avatar scanner at any time.
	 */
	static void toggle(void*);

	static void showInstance();

	/**
	 * @brief Updates the internal avatar list with the currently present avatars.
	 */
	void updateAvatarList();

	/**
	 * @brief Refresh avatar list (display)
	 */
	void refreshAvatarList();

	/**
	 * @brief Returns the entry for an avatar, if preset
	 * @returns Pointer to avatar entry, NULL if not found.
	 */
	LLAvatarListEntry* getAvatarEntry(LLUUID avatar);

	/**
	 * @brief Returns a string with the selected names in the list
	 */
	std::string getSelectedNames(const std::string& separator = ", ");
	std::string getSelectedName();
	LLUUID getSelectedID();

private:
	static LLFloaterAvatarList* sInstance;

public:
	static LLFloaterAvatarList* getInstance() { return sInstance; }
private:
	// when a line editor loses keyboard focus, it is committed.
	// commit callbacks are named onCommitWidgetName by convention.
	//void onCommitBaz(LLUICtrl* ctrl, void *userdata);
	
	enum AVATARS_COLUMN_ORDER
	{
		LIST_MARK,
		LIST_AVATAR_NAME,
		LIST_DISTANCE,
		LIST_POSITION,
		LIST_ALTITUDE
	};

	typedef void (*avlist_command_t)(const LLUUID &avatar, const std::string &name);

	/**
	 * @brief Removes focus status from all avatars in list
	 */
	void removeFocusFromAll();

	/**
	 * @brief Focus camera on current avatar
	 */
	void focusOnCurrent();

	/**
	 * @brief Focus camera on previous avatar
	 * @param marked_only Whether to choose only marked avatars
	 */
	void focusOnPrev(BOOL marked_only);

	/**
	 * @brief Focus camera on next avatar
	 * @param marked_only Whether to choose only marked avatars
	 */
	void focusOnNext(BOOL marked_only);

	/**
	 * @brief Handler for the "refresh" button click.
	 * I am unsure whether this is actually necessary at the time.
	 *
	 * LL: By convention, button callbacks are named onClickButtonLabel
	 * @param userdata Pointer to user data (LLFloaterAvatarList instance)
	 */

	static void onClickProfile(void *userdata);
	static void onClickIM(void *userdata);
	static void onClickTeleportOffer(void *userdata);
	static void onClickTrack(void *userdata);
	static void onClickMark(void *userdata);
	static void onClickFocus(void *userdata);

	static void onClickPrevInList(void *userdata);
	static void onClickNextInList(void *userdata);
	static void onClickPrevMarked(void *userdata);
	static void onClickNextMarked(void *userdata);
	static void onClickGetKey(void *userdata);

	static void onClickFreeze(void *userdata);
	static void onClickEject(void *userdata);
	static void onClickMute(void *userdata);
	static void onClickAR(void *userdata);
	static void onClickTeleport(void *userdata);
	static void onClickEjectFromEstate(void *userdata);

	static void callbackFreeze(const LLSD& notification, const LLSD& response);
	static void callbackEject(const LLSD& notification, const LLSD& response);
	static void callbackAR(void *userdata);
	static void callbackEjectFromEstate(const LLSD& notification, const LLSD& response);

	static void onSelectName(LLUICtrl*, void *userdata);

	static void onCommitUpdateRate(LLUICtrl*, void *userdata);
	static void onClickSendKeys(void *userdata);

	static void callbackIdle(void *userdata);

	void doCommand(avlist_command_t cmd);

	/**
	 * @brief Cleanup avatar list, removing dead entries from it.
	 * This lets dead entries remain for some time. This makes it possible
	 * to keep people passing by in the list long enough that it's possible
	 * to do something to them.
	 */
	void expireAvatarList();

private:
	/**
	 * @brief Pointer to the avatar scroll list
	 */
	LLScrollListCtrl*			mAvatarList;
	std::map<LLUUID, LLAvatarListEntry>	mAvatars;

	/**
	 * @brief TRUE when Updating
	 */
	BOOL mUpdate;

	/**
	 * @brief Update rate (if min frames per update)
	 */
	U32 mUpdateRate;
	
	void stopTracker();
	void refreshTracker();

	// tracking data
	BOOL mTracking;			// Tracking ?
	LLUUID mTrackedAvatar;	// Who we are tracking

	/**
	 * @brief Avatar the camera is focused on
	 */
	LLUUID mFocusedAvatar;
};
