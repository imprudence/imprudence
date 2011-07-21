/**
* @file floatercache.cpp
* @brief clear cache window for Imprudence
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2011, McCabe Maxsted
*
* Imprudence Viewer Source Code
* The source code in this file ("Source Code") is provided to you
* under the terms of the GNU General Public License, version 2.0
* ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
* this distribution, or online at
* http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
* ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
* WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
* COMPLETENESS OR PERFORMANCE.
* $/LicenseInfo$
*/

#include "llviewerprecompiledheaders.h"

#include "floatercache.h"

#include "llcheckboxctrl.h"
#include "lluictrlfactory.h"

#include "llviewercontrol.h"


FloaterCache::FloaterCache(const LLSD& seed) : LLFloater("Clear Cache")
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_clear_cache.xml");
}

FloaterCache::~FloaterCache()
{
}

BOOL FloaterCache::postBuild()
{
	getChild<LLCheckBoxCtrl>("texture_cache")->setValue(gSavedSettings.getBOOL("ClearTextureCache"));
	getChild<LLCheckBoxCtrl>("object_cache")->setValue(gSavedSettings.getBOOL("ClearObjectCache"));
	getChild<LLCheckBoxCtrl>("inv_cache")->setValue(gSavedSettings.getBOOL("ClearInvCache"));
	getChild<LLCheckBoxCtrl>("name_cache")->setValue(gSavedSettings.getBOOL("ClearNameCache"));
	getChild<LLCheckBoxCtrl>("sounds_cache")->setValue(gSavedSettings.getBOOL("ClearSoundsCache"));

	childSetAction("btn_ok", onClickOK, this);
	childSetAction("btn_cancel", onClickCancel, this);

	return TRUE;
}

// static
void FloaterCache::onClickOK(void* data)
{
	FloaterCache* self = (FloaterCache*)data;
	if (self)
	{
		bool purge_textures = self->getChild<LLCheckBoxCtrl>("texture_cache")->getValue().asBoolean();
		bool purge_objects = self->getChild<LLCheckBoxCtrl>("object_cache")->getValue().asBoolean();
		bool purge_inv = self->getChild<LLCheckBoxCtrl>("inv_cache")->getValue().asBoolean();
		bool purge_names = self->getChild<LLCheckBoxCtrl>("name_cache")->getValue().asBoolean();
		bool purge_sounds = self->getChild<LLCheckBoxCtrl>("sounds_cache")->getValue().asBoolean();
		
		gSavedSettings.setBOOL("ClearTextureCache", purge_textures);
		gSavedSettings.setBOOL("ClearObjectCache", purge_objects);
		gSavedSettings.setBOOL("ClearInvCache", purge_inv);
		gSavedSettings.setBOOL("ClearNameCache", purge_names);
		gSavedSettings.setBOOL("ClearSoundsCache", purge_sounds);
			
		if (purge_textures || purge_objects || purge_inv || purge_names || purge_sounds)
		{
			// flag client cache for clearing next time the client runs
			gSavedSettings.setBOOL("PurgeCacheOnNextStartup", TRUE);
		}

		self->close();
	}
}

// static
void FloaterCache::onClickCancel(void* data)
{
	FloaterCache* self = (FloaterCache*)data;
	if (self)
	{
		self->close();
	}
}
