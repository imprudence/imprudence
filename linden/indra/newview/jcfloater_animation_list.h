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

#include "llfloater.h"
#include "lluuid.h"
#include "llstring.h"

struct AObjectData
{
	std::string owner_name;
	LLUUID owner_id;
	LLUUID root_id;
	bool in_object_list;
};

class JCFloaterAnimList : public LLFloater, public LLUISingleton<JCFloaterAnimList, JCFloaterAnimList>
{
public:
	JCFloaterAnimList(const LLSD& seed);
	virtual ~JCFloaterAnimList();

	/*virtual*/ BOOL postBuild();
	/*virtual*/ void draw();
	/*virtual*/ void onVisibilityChange(BOOL new_visibility);

	/*virtual*/ void close(bool app = 0);
	 void refresh();

	const LLUUID& findItemID(const LLUUID& asset_id, BOOL copyable_only);

	static void callbackLoadOwnerName(const LLUUID& id, const std::string& first, const std::string& last, BOOL is_group, void* data);

	static void processObjectPropertiesFamily(LLMessageSystem* msg, void** user_data);

	static bool visible(LLFloater* instance, const LLSD& key);
	static void show(LLFloater* instance, const LLSD& key);
	static void hide(LLFloater* instance, const LLSD& key);

private:

	enum ANIMATION_COLUMN_ORDER
	{
		LIST_ANIMATION_NAME,
		LIST_ANIMATION_UUID,
		LIST_OBJECT_UUID,
		LIST_OBJECT_OWNER
	};

	LLScrollListCtrl* mAnimList;

	std::map<LLUUID, AObjectData> mObjectOwners;

	static void StopSelected(void *userdata );
	static void RevokeSelected(void *userdata );
	static void StopRevokeSelected(void *userdata );
	static void OpenProfile(void *userdata );
	static void onDoubleClick(void *userdata);

};

