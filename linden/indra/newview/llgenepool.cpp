/** 
 * @file llgenepool.cpp
 * @brief LLGenePool class implementation
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

#include "llgenepool.h"
#include "llxmltree.h"
#include "llvoavatar.h"
#include "lluuid.h"
#include "llagent.h"
#include "llviewervisualparam.h"
#include "llxmltree.h"
#include "llviewerimagelist.h"
#include "llappearance.h"

#include "lldir.h"

LLGenePool::LLGenePool()
	:
	mLoaded( FALSE )
{
}

LLGenePool::~LLGenePool()
{
	for( S32 i = 0; i < mArchetypes.count(); i++ )
	{
		delete mArchetypes[i];
	}
}


BOOL LLGenePool::load()
{
	std::string filename;

	filename = gDirUtilp->getExpandedFilename(LL_PATH_CHARACTER,"genepool.xml");
	if( mLoaded )
	{
		return TRUE;
	}

	LLXmlTree xml_tree;
	BOOL success = xml_tree.parseFile( filename, FALSE );
	if( !success )
	{
		return FALSE;
	}

	LLXmlTreeNode* root = xml_tree.getRoot();
	if( !root )
	{
		return FALSE;
	}

	//-------------------------------------------------------------------------
	// <linden_genepool version="1.0"> (root)
	//-------------------------------------------------------------------------
	if( !root->hasName( "linden_genepool" ) )
	{
		llwarns << "Invalid linden_genepool file header: " << filename << llendl;
		return FALSE;
	}

	std::string version;
	static LLStdStringHandle version_string = LLXmlTree::addAttributeString("version");
	if( !root->getFastAttributeString( version_string, version ) || (version != "1.0") )
	{
		llwarns << "Invalid linden_genepool file version: " << version << llendl;
		return FALSE;
	}

	//-------------------------------------------------------------------------
	// <archetype>
	//-------------------------------------------------------------------------
	for (LLXmlTreeNode* child = root->getChildByName( "archetype" );
		 child;
		 child = root->getNextNamedChild())
	{
		if( !loadNodeArchetype( child ) )
		{
			return FALSE;
		}
	}

	mLoaded = TRUE;
	return TRUE;
}


//-----------------------------------------------------------------------------
// loadNodeArchetype(): loads <archetype> node from XML tree
//-----------------------------------------------------------------------------
BOOL LLGenePool::loadNodeArchetype( LLXmlTreeNode* node )
{
	llassert( node->hasName( "archetype" ) );

	LLAppearance* archetype = new LLAppearance();
	BOOL success = TRUE;

	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if( !avatar )
	{
		delete archetype;
		return FALSE;
	}

	LLXmlTreeNode* child;
	for (child = node->getChildByName( "param" );
		 child;
		 child = node->getNextNamedChild())
	{
		F32 value;
		static LLStdStringHandle value_string = LLXmlTree::addAttributeString("value");
		if( !child->getFastAttributeF32( value_string, value ) )
		{
			llwarns << "avatar genepool file: <param> missing value attribute" << llendl;
			success = FALSE;
			break;
		}
			
		S32 id;
		static LLStdStringHandle id_string = LLXmlTree::addAttributeString("id");
		if( !child->getFastAttributeS32( id_string, id ) )
		{
			llwarns << "avatar genepool file: <param> missing id attribute" << llendl;
			success = FALSE;
			break;
		}

		LLVisualParam* param = avatar->getVisualParam( id );
		if( param )
		{
			archetype->addParam( id, value );
		}
		else
		{
			llwarns << "avatar genepool file: ignoring invalid <param> with id: " << id << llendl;
		}
	}
	for (child = node->getChildByName( "texture" );
		 child;
		 child = node->getNextNamedChild())
	{
		LLUUID uuid;
		static LLStdStringHandle uuid_string = LLXmlTree::addAttributeString("uuid");
		if( !child->getFastAttributeUUID( uuid_string, uuid ) )
		{
			llwarns << "avatar genepool file: <texture> missing uuid attribute" << llendl;
			success = FALSE;
			break;
		}
			
		S32 te;
		static LLStdStringHandle te_string = LLXmlTree::addAttributeString("te");
		if( !child->getFastAttributeS32( te_string, te ) )
		{
			llwarns << "avatar genepool file: <texture> missing te attribute" << llendl;
			success = FALSE;
			break;
		}

		archetype->addTexture( te, uuid );
	}

	if( success )
	{
		mArchetypes.put( archetype );
	}
	else
	{
		delete archetype;
	}
	return success;
}

// Creates new parameters for the given wearable and applies them to the avatar.
void LLGenePool::spawn( EWearableType type )
{
	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if( !avatar )
	{
		return;
	}
	
	if( !mLoaded )
	{
		if( !load() )
		{
			return;
		}
	}

	if( mArchetypes.count() < 1 )
	{
		return;
	}

	// Only consider archetypes that have the same sex as you have already.
	LLVisualParam* male_param = avatar->getVisualParam( "male" );

	if (!male_param)
	{
		llwarns << "The hard coded \'male\' parameter passed to avatar->getVisualParam() in LLGenePool::spawn() is no longer valid."
				<< llendl;
		return;
	}

	S32 male_param_id = male_param->getID();
	F32 sex_weight = male_param->getWeight();

	S32 i = 0;
	S32 j = 0;
	S32 k = 0;
	const S32 MAX_CYCLES = 1000;
	S32 cycles = 0;

	F32 cur_sex_weight = 0.f;
	do
	{
		i = rand() % mArchetypes.count();
		cur_sex_weight = mArchetypes[i]->getParam(male_param_id, 0.f);
		cycles++;
		if (cur_sex_weight != sex_weight)
		{
			break;
		}
	} while((cycles < MAX_CYCLES));

	if( cycles >= MAX_CYCLES )
	{
		return;
	}

	LLAppearance* arch1 = mArchetypes[i];
	LLAppearance* arch2 = NULL;
	LLAppearance* arch3 = NULL;

	if( mArchetypes.count() > 1 )
	{
		cycles = 0;
    	do
    	{
    		j = rand() % mArchetypes.count();
			cur_sex_weight = mArchetypes[j]->getParam(male_param_id, 0.f);

			cycles++;
		} while( 
			(cycles < MAX_CYCLES) && 
			( (i == j) || (cur_sex_weight != sex_weight) )
		);

		if( cycles >= MAX_CYCLES )
		{
			return;
		}

    	arch2 = mArchetypes[j];
	}

	if( mArchetypes.count() > 2 )
	{
		cycles = 0;
    	do
    	{
    		k = rand() % mArchetypes.count();
			cur_sex_weight = mArchetypes[k]->getParam(male_param_id, 0.f);
			cycles++;
		} while( 
			(cycles < MAX_CYCLES) && 
			( (i == k) || (j == k) || (cur_sex_weight != sex_weight) )
		);

		if( cycles >= MAX_CYCLES )
		{
			return;
		}

    	arch3 = mArchetypes[k];
	}

	// Lame generation of barycentric coordinates
	F32 b1 = F32( rand() ) / RAND_MAX;
	F32 b2 = (F32( rand() ) / RAND_MAX) * (1.f - b1);
	F32 b3 = 1.f - b1 - b2;

//	ESex old_sex = avatar->getSex();

	// Pull params
	for( LLVisualParam* param = avatar->getFirstVisualParam(); param; param = avatar->getNextVisualParam() )
	{
		if( (((LLViewerVisualParam*)param)->getWearableType() == type) && (param->getGroup() == VISUAL_PARAM_GROUP_TWEAKABLE) )
		{
			S32 param_id = param->getID();

			// don't try to blend male/female param...as fp innaccuracy will break the [0 | 1] semantics
			if (param_id != male_param_id)
			{

				F32 weight1 = arch1->getParam( param_id, param->getDefaultWeight() );
				F32 net_weight = weight1;
				
				if( arch2 && arch3 )
				{
    				F32 weight2 = arch2->getParam( param_id, param->getDefaultWeight() );
    				F32 weight3 = arch3->getParam( param_id, param->getDefaultWeight() );
					net_weight = b1 * weight1 + b2 * weight2 + b3 * weight3;
				}
			
				param->setAnimationTarget(net_weight, TRUE);
			}
		}
	}

	// Find the archetype with the greatest influence
	LLAppearance* dominant_arch = arch1;
	if( (b2 > b1) && (b2 > b3) )
	{
		dominant_arch = arch2;
	}
	else
	if( (b3 > b1) && (b3 > b2) )
	{
		dominant_arch = arch3;
	}


	// Pull Textures from the dominant archetype
	for( S32 te = 0; te < LLVOAvatar::TEX_NUM_ENTRIES; te++ )
	{
		if( LLVOAvatar::isTextureIndexBaked( te ) )
		{
			continue;
		}

		if( LLVOAvatar::getTEWearableType( te ) == type )
		{
			LLUUID image_id = dominant_arch->getTexture( te );
			if( image_id.isNull() )
			{
				image_id = LLVOAvatar::getDefaultTEImageID( te );
			}

			LLViewerImage* image = gImageList.getImage( image_id );
			if( image )
			{
				avatar->setLocTexTE( te, image, TRUE );
			}
		}
	}

//	avatar->setVisualParamWeight( "male", sex_weight );

	avatar->startAppearanceAnimation(TRUE, TRUE);
	avatar->updateVisualParams();

// 	ESex new_sex = avatar->getSex();
//	if( old_sex != new_sex )
//	{
//		avatar->updateSexDependentLayerSets( TRUE );
//	}	
	
	avatar->updateMeshTextures();
    gAgent.sendAgentSetAppearance();
}
