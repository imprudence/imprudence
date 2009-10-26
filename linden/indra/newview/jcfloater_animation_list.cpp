/* Copyright (c) 2009
 *
 * Modular Systems Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *   3. Neither the name Modular Systems Ltd nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MODULAR SYSTEMS LTD AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MODULAR SYSTEMS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "llviewerprecompiledheaders.h"

#include "lluuid.h"
#include "lluictrlfactory.h"
#include "llvoavatar.h"

#include "llagent.h"

#include "llscrolllistctrl.h"

#include "llviewerobjectlist.h"

#include "jcfloater_animation_list.h"

#include "llviewercontrol.h"

#include "llinventorymodel.h"

#include "llcategory.h"

#include "llfloaterchat.h"

#include "llfloateravatarinfo.h"





//std::map<LLUUID, AObjectData>	JCFloaterAnimList::mObjectOwners;

JCFloaterAnimList::JCFloaterAnimList(const LLSD& seed) : 
	LLFloater(std::string("animation list")),
	mAnimList(0)
{
	BOOL no_open = FALSE;
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_animation_list.xml",&getFactoryMap(),no_open);
}

JCFloaterAnimList::~JCFloaterAnimList()
{
}

//static 
bool JCFloaterAnimList::visible(LLFloater* instance, const LLSD& key)
{
	return VisibilityPolicy<LLFloater>::visible(instance, key);
}

//static 
void JCFloaterAnimList::show(LLFloater* instance, const LLSD& key)
{
	VisibilityPolicy<LLFloater>::show(instance, key);
}

//static 
void JCFloaterAnimList::hide(LLFloater* instance, const LLSD& key)
{
	VisibilityPolicy<LLFloater>::hide(instance, key);
}

void JCFloaterAnimList::onVisibilityChange(BOOL new_visibility)
{
	if(!new_visibility) {
		// *HACK: clean up memory on hiding
		mObjectOwners.clear();
	}
}

BOOL JCFloaterAnimList::postBuild()
{
	mAnimList = getChild<LLScrollListCtrl>("animation_list");
	mAnimList->setCallbackUserData(this);
	mAnimList->setDoubleClickCallback(onDoubleClick);
	mAnimList->sortByColumn("animation_uuid", TRUE);

	childSetAction("Stop Selected",StopSelected,this);
	childSetAction("Revoke Selected",RevokeSelected,this);
	childSetAction("Stop+Revoke Selected",StopRevokeSelected,this);
	childSetAction("Open Owner Profile",OpenProfile,this);

	return 1;
}

void JCFloaterAnimList::StopSelected(void *userdata )
{
	JCFloaterAnimList *self = (JCFloaterAnimList*)userdata;
	LLDynamicArray<LLUUID> ids;
	std::vector< LLScrollListItem * > items = self->mAnimList->getAllSelected();
	for( std::vector< LLScrollListItem * >::iterator itr = items.begin(); itr != items.end(); itr++ )
	{
		LLScrollListItem *item = *itr;
		const LLUUID &id = item->getColumn(LIST_ANIMATION_UUID)->getValue().asUUID();
		if( ids.find(id) == LLDynamicArray<LLUUID>::FAIL )
		{
			ids.put(id);
		}
	}
	gAgent.sendAnimationRequests(ids,ANIM_REQUEST_STOP);
}

void JCFloaterAnimList::RevokeSelected(void *userdata )
{
	JCFloaterAnimList *self = (JCFloaterAnimList*)userdata;
	LLDynamicArray<LLUUID> ids;
	std::vector< LLScrollListItem * > items = self->mAnimList->getAllSelected();
	for( std::vector< LLScrollListItem * >::iterator itr = items.begin(); itr != items.end(); itr++ )
	{
		LLScrollListItem *item = *itr;
		const LLUUID &id = item->getColumn(LIST_OBJECT_UUID)->getValue().asUUID();
		if( ids.find(id) == LLDynamicArray<LLUUID>::FAIL )
		{
			ids.put(id);
		}
	}
	if( !ids.empty() )
	{
		for(LLDynamicArray<LLUUID>::iterator itr = ids.begin(); itr != ids.end(); ++itr)
		{
			LLUUID id = *itr;
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_RevokePermissions);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_Data);
			msg->addUUIDFast(_PREHASH_ObjectID, id);
			msg->addU32Fast(_PREHASH_ObjectPermissions, U32_MAX);
			gAgent.sendReliableMessage();
		}
	}
}

void JCFloaterAnimList::StopRevokeSelected(void *userdata )
{
	StopSelected(userdata);
	RevokeSelected(userdata);
}

void JCFloaterAnimList::OpenProfile(void *userdata )
{
	JCFloaterAnimList *self = (JCFloaterAnimList*)userdata;
	std::vector< LLScrollListItem * > items = self->mAnimList->getAllSelected();
	if(!items.empty())
	{
		//LLVOAvatar* avatarp = gAgent.getAvatarObject();	
		for(std::vector< LLScrollListItem * >::iterator itr = items.begin(); itr != items.end(); ++itr)
		{
			LLScrollListItem *item = *itr;
			const LLUUID &object_id = item->getColumn(LIST_OBJECT_UUID)->getValue().asUUID();
			LLFloaterAvatarInfo::showFromDirectory(self->mObjectOwners[object_id].owner_id);
		}
	}
}

/*void JCFloaterAnimList::ReturnSelected(void *userdata )
{
	JCFloaterAnimList *self = (JCFloaterAnimList*)userdata;
	LLDynamicArray<LLUUID> ids = self->mAnimList->getSelectedIDs();
	if(ids.size() > 0)
	{
		LLVOAvatar* avatarp = gAgent.getAvatarObject();	
		for(LLDynamicArray<LLUUID>::iterator itr = ids.begin(); itr != ids.end(); ++itr)
		{
			LLUUID id = *itr;

		}
	}
}*/

void JCFloaterAnimList::draw()
{
	refresh();
	LLFloater::draw();
}
//LLScrollListCtrl::getSelectedIDs();
void JCFloaterAnimList::refresh()
{
	LLDynamicArray<LLUUID> selected = mAnimList->getSelectedIDs();
	S32 scrollpos = mAnimList->getScrollPos();
	mAnimList->deleteAllItems();
	LLVOAvatar* avatarp = gAgent.getAvatarObject();		
	if (avatarp)
	{
		LLVOAvatar::AnimSourceIterator ai;

		for(ai = avatarp->mAnimationSources.begin(); ai != avatarp->mAnimationSources.end(); ++ai)
		{
			LLSD element;
			const LLUUID &aifirst = ai->first;
			LLViewerInventoryItem* item = gInventory.getItem(findItemID(ai->second,0));

			// *NOTE: conceal id to prevent bugs, use
			// item->getColumn(LIST_ANIMATION_UUID)->getValue().asUUID()
			// instead
			element["id"] = LLUUID::null.combine(ai->second);
			element["columns"][LIST_ANIMATION_NAME]["column"] = "Anim Name";
			element["columns"][LIST_ANIMATION_NAME]["type"] = "text";
			element["columns"][LIST_ANIMATION_NAME]["color"] = gColors.getColor("ScrollUnselectedColor").getValue();
			if(item)
			{
				element["columns"][LIST_ANIMATION_NAME]["value"] = item->getName();//ai->second//"avatar_icon";
			}else
			{
				element["columns"][LIST_ANIMATION_NAME]["value"] = "Not in Inventory";
			}
			element["columns"][LIST_ANIMATION_UUID]["column"] = "Animation UUID";
			element["columns"][LIST_ANIMATION_UUID]["type"] = "text";
			element["columns"][LIST_ANIMATION_UUID]["color"] = gColors.getColor("ScrollUnselectedColor").getValue();
			element["columns"][LIST_ANIMATION_UUID]["value"] = ai->second;
			element["columns"][LIST_OBJECT_UUID]["column"] = "Source Object UUID";
			element["columns"][LIST_OBJECT_UUID]["type"] = "text";
			element["columns"][LIST_OBJECT_UUID]["color"] = gColors.getColor("ScrollUnselectedColor").getValue();
			element["columns"][LIST_OBJECT_UUID]["value"] = aifirst;
			element["columns"][LIST_OBJECT_OWNER]["column"] = "Source Owner";
			element["columns"][LIST_OBJECT_OWNER]["type"] = "text";
			element["columns"][LIST_OBJECT_OWNER]["color"] = gColors.getColor("ScrollUnselectedColor").getValue();
			std::string name("?");
			LLViewerObject *object = gObjectList.findObject(aifirst);
			bool is_first = ( mObjectOwners.count( aifirst ) == 0 );
			bool just_shown = false;
			LLUUID owner_id(LLUUID::null);

			if( !is_first )
			{
				name = mObjectOwners[aifirst].owner_name;
				owner_id = mObjectOwners[aifirst].owner_id;
			}

			if( object )
			{
				if( object->permYouOwner() )
				{
					owner_id = gAgent.getID();
					gAgent.getName(name);
				}
				else
				{
					object = (LLViewerObject *) object->getRoot();
					if( object->isAvatar() )
					{
						owner_id = object->getID();
						name = ((LLVOAvatar *)object)->getFullname();
					}
				}
			}

			{
				AObjectData &data = mObjectOwners[aifirst];
				if( object )
				{
					if( !data.in_object_list )
					{
						just_shown = true;
						data.in_object_list = true;
					}
					data.root_id = ( (LLViewerObject*)object->getRoot() )->getID();
				}
				data.owner_name = name;
				data.owner_id = owner_id;
			}

			if( is_first || just_shown ) {
				if( name == "?" && !aifirst.isNull()) {
					LLMessageSystem* msg = gMessageSystem;
					msg->newMessageFast(_PREHASH_RequestObjectPropertiesFamily);
					msg->nextBlockFast(_PREHASH_AgentData);
					msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
					msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
					msg->nextBlockFast(_PREHASH_ObjectData);
					msg->addU32Fast(_PREHASH_RequestFlags, 0 );
					if( object )
					{
						LL_INFOS("Avatar List") << "Sending RequestObjectPropertiesFamily packet for id( " << aifirst.asString() << " ) on object( " << object->getID().asString() << " )" << LL_ENDL;
						msg->addUUIDFast(_PREHASH_ObjectID, object->getID());
					}
					else
					{
						LL_INFOS("Avatar List") << "Sending RequestObjectPropertiesFamily packet for id( " << aifirst.asString() << " )" << LL_ENDL;
						msg->addUUIDFast(_PREHASH_ObjectID, aifirst);
					}
					gAgent.sendReliableMessage();
				}
			}
			element["columns"][LIST_OBJECT_OWNER]["value"] = name;
			mAnimList->addElement(element, ADD_BOTTOM);
			//LLViewerObject* objectp = gObjectList.findObject(ai->first);
			//if(objectp)
			//{
			//	//objectp->
			//}

			//object_ids.insert(ai->first);
			//animation_ids.insert(ai->second);
		}
	}

	mAnimList->sortItems();
	mAnimList->selectMultiple(selected);
	mAnimList->setScrollPos(scrollpos);
}

void JCFloaterAnimList::callbackLoadOwnerName(const LLUUID& id, const std::string& first, const std::string& last, BOOL is_group, void* data)
{
	LLUUID *oid = (LLUUID*)data;
	JCFloaterAnimList *self = JCFloaterAnimList::getInstance(LLSD());
	if(self->mObjectOwners.count( *oid ) > 0)
	{
		self->mObjectOwners[*oid].owner_name = first + " " + last;
	}
	delete oid;
}

void JCFloaterAnimList::processObjectPropertiesFamily(LLMessageSystem* msg, void** user_data)
{
	if(!JCFloaterAnimList::instanceVisible(LLSD())) return;
	JCFloaterAnimList *self = JCFloaterAnimList::getInstance(LLSD());
	LLUUID object_id;
	U32 request_flags;
	LLUUID creator_id;
	LLUUID owner_id;
	LLUUID group_id;
	LLUUID extra_id;
	U32 base_mask, owner_mask, group_mask, everyone_mask, next_owner_mask;
	LLSaleInfo sale_info;
	LLCategory category;
	msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_RequestFlags,	request_flags );
	msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_ObjectID,		object_id );
	msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_OwnerID,		owner_id );
	msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_GroupID,		group_id );
	msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_BaseMask,		base_mask );
	msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_OwnerMask,		owner_mask );
	msg->getU32Fast(_PREHASH_ObjectData,_PREHASH_GroupMask,		group_mask );
	msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_EveryoneMask,	everyone_mask );
	msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_NextOwnerMask, next_owner_mask);
	sale_info.unpackMessage(msg, _PREHASH_ObjectData);
	category.unpackMessage(msg, _PREHASH_ObjectData);
	LLUUID last_owner_id;
	msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_LastOwnerID, last_owner_id );
	std::string name;
	msg->getStringFast(_PREHASH_ObjectData, _PREHASH_Name, name);
	std::string desc;
	msg->getStringFast(_PREHASH_ObjectData, _PREHASH_Description, desc);

	for( std::map<LLUUID, AObjectData>::iterator di = self->mObjectOwners.begin(); di != self->mObjectOwners.end(); di++ )
	{
		const LLUUID &id = di->first;
		const AObjectData &data = di->second;
		
		if(data.root_id == object_id || data.owner_id == object_id)
		{
			LLUUID* ref = new LLUUID(id);
			self->mObjectOwners[id].owner_id = owner_id;
			gCacheName->get(owner_id, FALSE, callbackLoadOwnerName, (void*)ref);
		}
	}
}

const LLUUID& JCFloaterAnimList::findItemID(const LLUUID& asset_id, BOOL copyable_only)
{
	LLViewerInventoryCategory::cat_array_t cats;
	LLViewerInventoryItem::item_array_t items;
	LLAssetIDMatches asset_id_matches(asset_id);
	gInventory.collectDescendentsIf(LLUUID::null,
							cats,
							items,
							LLInventoryModel::INCLUDE_TRASH,
							asset_id_matches);

	if (items.count())
	{
		// search for copyable version first
		for (S32 i = 0; i < items.count(); i++)
		{
			LLInventoryItem* itemp = items[i];
			LLPermissions item_permissions = itemp->getPermissions();
			if (item_permissions.allowCopyBy(gAgent.getID(), gAgent.getGroupID()))
			{
				return itemp->getUUID();
			}
		}
		// otherwise just return first instance, unless copyable requested
		if (copyable_only)
		{
			return LLUUID::null;
		}
		else
		{
			return items[0]->getUUID();
		}
	}

	return LLUUID::null;
}

/*void JCFloaterAnimList::onClickCopyAnimationUUID(void *userdata)
{
	LLFloaterAvatarList *self = (LLFloaterAvatarList*)userdata;
 	LLScrollListItem *item = self->mAvatarList->getFirstSelected();

	if ( NULL == item ) return;

	LLUUID agent_id = item->getUUID();

	char buffer[UUID_STR_LENGTH];		
	agent_id.toString(buffer);

	gViewerWindow->mWindow->copyTextToClipboard(utf8str_to_wstring(buffer));
}*/

void JCFloaterAnimList::onDoubleClick(void *userdata)
{
	//JCFloaterAnimList *self = (JCFloaterAnimList*)userdata;
 	//LLScrollListItem *item =   self->mAnimList->getFirstSelected();
	//if(!item)return;
	//LLUUID agent_id = item->getUUID();

	//gAgent.setFocusObject(gObjectList.findObject(agent_id));
	//gAgent.setFocusOnAvatar(FALSE, TRUE);
}

void JCFloaterAnimList::close(bool app)
{
	//sInstance->setVisible(0);
//#ifdef RECONSTRUCT_ON_TOGGLE
//	sInstance = NULL;
	LLFloater::close(app);
//#else
//	sInstance->setVisible(!(sInstance->getVisible()));
//#endif
}

