/** 
 * @file inventory.cpp
 * @author Phoenix
 * @date 2005-11-15
 * @brief Functions for inventory test framework
 *
 * Copyright (c) 2005-2007, Linden Research, Inc.
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

/** 
 * 
 * THOROUGH_DESCRIPTION of inventory.cpp
 *
 */

#include "linden_common.h"
#include "lltut.h"
#include "llinventory.h"
#include "llsd.h"

LLPointer<LLInventoryItem> create_random_inventory_item()
{
	LLUUID item_id;
	item_id.generate();
	LLUUID parent_id;
	parent_id.generate();
	LLPermissions perm;
	LLUUID creator_id;
	creator_id.generate();
	LLUUID owner_id;
	owner_id.generate();
	LLUUID last_owner_id;
	last_owner_id.generate();
	LLUUID group_id;
	group_id.generate();
	perm.init(creator_id, owner_id, last_owner_id, group_id);
	perm.initMasks(PERM_ALL, PERM_ALL, PERM_COPY, PERM_COPY, PERM_MODIFY | PERM_COPY);
	LLUUID asset_id;
	asset_id.generate();
	S32 price = rand();
	LLSaleInfo sale_info(LLSaleInfo::FS_COPY, price);
	U32 flags = rand();
	S32 creation = time(NULL);

	LLPointer<LLInventoryItem> item = new LLInventoryItem(
		item_id,
		parent_id,
		perm,
		asset_id,
		LLAssetType::AT_OBJECT,
		LLInventoryType::IT_ATTACHMENT,
		"Sample Object",
		"Used for Testing",
		sale_info,
		flags,
		creation);
	return item;
}

LLPointer<LLInventoryCategory> create_random_inventory_cat()
{
	LLUUID item_id;
	item_id.generate();
	LLUUID parent_id;
	parent_id.generate();

	LLPointer<LLInventoryCategory> cat = new LLInventoryCategory(
		item_id,
		parent_id,
		LLAssetType::AT_NONE,
		"Sample category");
	return cat;
}

namespace tut
{
	struct inventory_data
	{
	};
	typedef test_group<inventory_data> inventory_test;
	typedef inventory_test::object inventory_object;
	tut::inventory_test inv("llinventory");

	template<> template<>
	void inventory_object::test<1>()
	{
		LLPointer<LLInventoryItem> src = create_random_inventory_item();
		LLSD sd = ll_create_sd_from_inventory_item(src);
		//llinfos << "sd: " << *sd << llendl;
		LLPointer<LLInventoryItem> dst;
		dst = ll_create_item_from_sd(sd);
		ensure_equals("item id", dst->getUUID(), src->getUUID());
		ensure_equals("parent", dst->getParentUUID(), src->getParentUUID());
		ensure_equals("name", dst->getName(), src->getName());
		ensure_equals("type", dst->getType(), src->getType());
		ensure_equals(
			"permissions",
			dst->getPermissions(),
			src->getPermissions());
		ensure_equals(
			"description",
			dst->getDescription(),
			src->getDescription());
		ensure_equals(
			"sale type",
			dst->getSaleInfo().getSaleType(),
			src->getSaleInfo().getSaleType());
		ensure_equals(
			"sale price",
			dst->getSaleInfo().getSalePrice(),
			src->getSaleInfo().getSalePrice());
		ensure_equals("asset id", dst->getAssetUUID(), src->getAssetUUID());
		ensure_equals(
			"inventory type",
			dst->getInventoryType(),
			src->getInventoryType());
		ensure_equals("flags", dst->getFlags(), src->getFlags());
		ensure_equals(
			"creation",
			dst->getCreationDate(),
			src->getCreationDate());
	}

	template<> template<>
	void inventory_object::test<2>()
	{
		LLPointer<LLInventoryCategory> src = create_random_inventory_cat();
		LLSD sd = ll_create_sd_from_inventory_category(src);
		LLPointer<LLInventoryCategory> dst = ll_create_category_from_sd(sd);
		ensure_equals("item id", dst->getUUID(), src->getUUID());
		ensure_equals("parent", dst->getParentUUID(), src->getParentUUID());
		ensure_equals("name", dst->getName(), src->getName());
		ensure_equals("type", dst->getType(), src->getType());
		ensure_equals(
			"preferred type",
			dst->getPreferredType(),
			src->getPreferredType());
	}

/*
	template<> template<>
	void inventory_object::test<3>()
	{
			
	}
*/
}
