/** 
 * @file lltransactionflags.cpp
 * @brief Some exported symbols and functions for dealing with
 * transaction flags.
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

#include "linden_common.h"

#include "lltransactionflags.h"
 
const U8 TRANSACTION_FLAGS_NONE = 0;
const U8 TRANSACTION_FLAG_SOURCE_GROUP = 1;
const U8 TRANSACTION_FLAG_DEST_GROUP = 2;
const U8 TRANSACTION_FLAG_OWNER_GROUP = 4;
const U8 TRANSACTION_FLAG_SIMULTANEOUS_CONTRIBUTION = 8;
const U8 TRANSACTION_FLAG_SIMULTANEOUS_CONTRIBUTION_REMOVAL = 16;

U8 pack_transaction_flags(BOOL is_source_group, BOOL is_dest_group)
{
	U8 rv = 0;
	if(is_source_group) rv |= TRANSACTION_FLAG_SOURCE_GROUP;
	if(is_dest_group) rv |= TRANSACTION_FLAG_DEST_GROUP;
	return rv;
}

BOOL is_tf_source_group(TransactionFlags flags)
{
	return ((flags & TRANSACTION_FLAG_SOURCE_GROUP) == TRANSACTION_FLAG_SOURCE_GROUP);
}

BOOL is_tf_dest_group(TransactionFlags flags)
{
	return ((flags & TRANSACTION_FLAG_DEST_GROUP) == TRANSACTION_FLAG_DEST_GROUP);
}

BOOL is_tf_owner_group(TransactionFlags flags)
{
	return ((flags & TRANSACTION_FLAG_OWNER_GROUP) == TRANSACTION_FLAG_OWNER_GROUP);
}

