/** 
 * @file llvotreenew.cpp
 * @brief LLVOTreeNew class implementation
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
 * 
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

#include "llviewerprecompiledheaders.h"

#include "llvotreenew.h"

#include "llgl.h"
#include "llglheaders.h"
#include "lltree_common.h"
#include "lltreeparams.h"
#include "material_codes.h"
#include "object_flags.h"

#include "llagent.h"
#include "llagparray.h"
#include "llviewercontrol.h"
#include "llcylinder.h"
#include "lldrawable.h"
#include "llface.h"
#include "llprimitive.h"
#include "llviewerimagelist.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
#include "llworld.h"
#include "noise.h"
#include "pipeline.h"

U32 LLVOTreeNew::sNextVertexIndex[MAX_SPECIES];
U32 LLVOTreeNew::sNextIndiceIndex[MAX_SPECIES];
U32 LLVOTreeNew::sNextPartIndex[MAX_PARTS];
LLVOTreeNew::TreePart LLVOTreeNew::sTreeParts[MAX_SPECIES][MAX_PARTS];
LLUUID LLVOTreeNew::sTreeImageIDs[MAX_SPECIES];
U32 LLVOTreeNew::sTreePartsUsed[MAX_SPECIES][MAX_PARTS][MAX_VARS];
LLTreeParams LLVOTreeNew::sParameters;
F32 LLVOTreeNew::sRandNums[MAX_RAND_NUMS];

extern LLPipeline gPipeline;

// Tree variables and functions

LLVOTreeNew::LLVOTreeNew(const LLUUID &id, const LLPCode pcode, LLViewerRegion *regionp):
						LLViewerObject(id, pcode, regionp)
{
	mTotalIndices = 0;
	mTotalVerts = 0;
	mSpecies = 0;
}


LLVOTreeNew::~LLVOTreeNew()
{
	if (mData)
	{
		delete[] mData;
		mData = NULL;
	}
}


void LLVOTreeNew::initClass()
{
	U8 i;

	// for now just load the same texture in for every tree...
	
	for (i = 0; i < MAX_SPECIES; i++) 
	{
		switch (i) {
		case (0):
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_pine_1.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_pine_1.tga") ));
			break;
		case (1):
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_oak.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_oak.tga") ));
			break;
		case (2):
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_tropical_1.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_tropical_1.tga") ));
			break;
		case (3):
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_palm_1.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_palm_1.tga") ));
			break;
		case (4):
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_dogwood.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_dogwood.tga") ));
			break;	
		case (5):
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_tropical_2.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_tropical_2.tga") ));
			break;	
		case (6):
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_palm_2.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_palm_2.tga") ));
			break;			
		case (7):
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_cypress_1.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_cypress_1.tga") ));
			break;	
		case (8):
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_cypress_2.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_cypress_2.tga") ));
			break;	
		default:
			sTreeImageIDs[i] = LLUUID( gViewerArt.getString("tree_pine_2.tga") );
			//LLVOTreeNew::sTreeImagep[i] = gImageList.getImage(LLUUID( gViewerArt.getString("tree_pine_2.tga") ));
			break;			
		}
	}

	
	//LLVOTreeNew::sParameters = LLTreeParams();

	// initialize an array of random numbers that we'll be using
	gLindenLabRandomNumber.seed(0);
	for (i = 0; i < MAX_RAND_NUMS; i++)
		sRandNums[i] = gLindenLabRandomNumber.llfrand(1.0);
}

/*
void LLVOTreeNew::cleanupTextures()
{
	S32 i;
	for (i = 0; i < MAX_SPECIES; i++)
	{
		mTreeImagep = NULL;
	}
}
*/

U32 LLVOTreeNew::processUpdateMessage(LLMessageSystem *mesgsys,
										  void **user_data,
										  U32 block_num, EObjectUpdateType update_type,
										  LLDataPacker *dp)
{
	// Do base class updates...
	U32 retval = LLViewerObject::processUpdateMessage(mesgsys, user_data, block_num, update_type, dp);

	if (  (getVelocity().magVecSquared() > 0.f)
		||(getAcceleration().magVecSquared() > 0.f)
		||(getAngularVelocity().magVecSquared() > 0.f))
	{
		llinfos << "ACK! Moving tree!" << llendl;
		setVelocity(LLVector3::zero);
		setAcceleration(LLVector3::zero);
		setAngularVelocity(LLVector3::zero);
	}

	mSpecies = ((U8 *)mData)[0];
	mSpecies = 1;

	if (mSpecies >= MAX_SPECIES)
	{
		mSpecies = 0;
	}


	mTreeImagep = gImageList.getImage(sTreeImageIDs[mSpecies]);
	if (mTreeImagep)
	{
		mTreeImagep->bindTexture(0);
		mTreeImagep->setClamp(TRUE, TRUE);
	}

	return retval;
}


BOOL LLVOTreeNew::idleUpdate(LLAgent &agent, LLWorld &world, const F64 &time)
{
	return TRUE;
}


void LLVOTreeNew::render(LLAgent &agent)
{
	// nothing
}


void LLVOTreeNew::updateTextures(LLAgent &agent)
{
	LLVector3 position_local = getPositionAgent() - agent.getCameraPositionAgent();
	F32 dot_product = position_local * agent.getFrameAgent().getAtAxis();
	F32 cos_angle = dot_product / position_local.magVec();

	if (cos_angle > 1.f)
	{
		cos_angle = 1.f;
	}

	// temporary aggregate texture scale calculation
	// this is less correct than the code above!
	F32 texel_area_ratio = 0.5f; // When we're billboarded, we use less than the whole texture (about half?)
	if (mTreeImagep)
	{
		mTreeImagep->addTextureStats(mPixelArea, texel_area_ratio, cos_angle);
	}

}


LLDrawable* LLVOTreeNew::createDrawable(LLPipeline *pipeline)
{
	
	pipeline->allocDrawable(this);
	mDrawable->setLit(FALSE);

	mDrawable->setRenderType(LLPipeline::RENDER_TYPE_TREE);
	//llinfos << "tree type: " << LLPipeline::RENDER_TYPE_TREE << llendl;

	//!! add the leaf texture to this
	LLDrawPool *poolp = gPipeline.getPool(LLDrawPool::POOL_TREE_NEW, mTreeImagep);


	//!! add a face for the bark texture...
	//LLFace *facep = mDrawable->addFace(poolp, mTreeImagep, TRUE);
	mDrawable->addFace(poolp, mTreeImagep, TRUE);
	//facep->setSize(1, 3);

	//!! add a face for the leaf texture ??
	//

	gPipeline.markMaterialed(mDrawable);

	return mDrawable;
}



BOOL LLVOTreeNew::updateGeometry(LLDrawable *drawable)
{

	llinfos << "TREE SPECIES " << U32(mSpecies) << llendl;

	// create the basic geometry of the trunk and branch

	// get the face for the branches...
	LLFace *face = drawable->getFace(0);
	LLDrawPool *poolp = face->getPool();

	if (!face->getDirty())
	{
		return TRUE;
	}

	poolp->setDirty();

	U32 curVertexIndex, curTexCoordIndex, curNormalIndex;
	U32 curIndiceIndex;
//	U32 numVertsCreated = LLVOTreeNew::sNextVertexIndex[mSpecies];

	//U32 vertexCount = poolp->getVertexCount();
	//llinfos << "TREE vertexCount = " << vertexCount << ", nextVertexIndex = " << mNextVertexIndex[mSpecies] << llendl;
	//U32 indiceCount = poolp->getIndexCount();
	//llinfos << "TREE indiceCount = " << indiceCount << ", nextIndiceIndex = " << mNextIndiceIndex[mSpecies] << llendl;


	if (poolp->getVertexCount() > 0)
	{	
		//llinfos << "TREE not creating the arrays..." << llendl;	
	}
	else
	{
		// create the drawpool arrays
		// using some rough estimates
		U32 numVerts = NUM_INIT_VERTS;
		U32 numIndices = NUM_INIT_INDICES;

		face->setPrimType(LLTriangles);
		face->setSize(numVerts, numIndices);

		// reset the next's since they may have been previously initialized...
		LLVOTreeNew::sNextIndiceIndex[mSpecies] = 0;
		LLVOTreeNew::sNextVertexIndex[mSpecies] = 0;
		LLVOTreeNew::sNextPartIndex[mSpecies] = 0;

		//llinfos << "TREE creating the arrays..." << llendl;
	}

	curVertexIndex = LLVOTreeNew::sNextVertexIndex[mSpecies];
	curTexCoordIndex = curVertexIndex;
	curNormalIndex = curVertexIndex;
	curIndiceIndex = LLVOTreeNew::sNextIndiceIndex[mSpecies];

	//vertexCount = poolp->getVertexCount();
	//llinfos << "TREE vertexCount = " << vertexCount << ", nextVertexIndex = " << mNextVertexIndex[mSpecies] << llendl;
	//indiceCount = poolp->getIndexCount();
	//llinfos << "TREE indiceCount = " << indiceCount << ", nextIndiceIndex = " << mNextIndiceIndex[mSpecies] << llendl;

	//llinfos << "TREE \t" << "\t curVertexIndex: \t" << curVertexIndex << llendl;
	//llinfos << "TREE \t" << "\t curIndiceIndex: \t" << curIndiceIndex << llendl;

	LLStrider<LLVector3> vertices;
	LLStrider<LLVector3> normals;
	LLStrider<LLVector2> tex_coords;
	U32 *indices;

	face->getGeometry(vertices, normals, tex_coords, indices);
	
	// create different parts for the levels...

	// create the trunk...
	F32 length, curve, radius, parentLength, parentRadius;
	
	length = parentLength = sParameters.mLength[0] * sParameters.mScale;
	radius = parentRadius = length * sParameters.mRatio * sParameters.mScale0;
	curve = sParameters.mCurve[0];
	createPart(0, length, radius, vertices, normals, tex_coords, indices, curVertexIndex, curTexCoordIndex,
			curNormalIndex, curIndiceIndex);

	// create collection of curved branches...
	//*
	for (U8 i = 1; i <= sParameters.mLevels; i++)
	{
		length = sParameters.mLength[i] * parentLength;
		radius = parentRadius * pow((length / parentLength), sParameters.mRatioPower);

		llinfos << "TREE creating (?) part with radius: " << radius << " and length: " << length << llendl;

		createPart(i, length, radius, vertices, normals, tex_coords, indices, curVertexIndex, curTexCoordIndex,
				curNormalIndex, curIndiceIndex);
		
		parentRadius = radius;
		parentLength = length;
		
	}
	//*/
	
	LLVOTreeNew::sNextVertexIndex[mSpecies] = curVertexIndex;
	LLVOTreeNew::sNextIndiceIndex[mSpecies] = curIndiceIndex;

	// create a leaf...?

	return TRUE;	
}

F32 LLVOTreeNew::CalcZStep(TreePart *part, U8 section)
{
	F32 zStep;

	if (part->mLevel == 0 && section < sParameters.mFlareRes)
		zStep = (sParameters.mFlarePercentage * part->mLength)/sParameters.mFlareRes;
	else
		zStep = ((1.f - sParameters.mFlarePercentage)*part->mLength) / (part->mCurveRes);

	return zStep;
}

S32 LLVOTreeNew::findSimilarPart(U8 level)
{
	// see if there's a tree part that's similar...
	for (U8 i = 0; i < sNextPartIndex[mSpecies]; i++)
	{
		TreePart *curPart = &(sTreeParts[mSpecies][i]);
		// make sure older branch is at same 'level' and has
		// similar lobe parameters (if at the trunk level)
		if (curPart->mLevel == level && 
			(level != 0 || (llabs(curPart->mLobes - sParameters.mLobes) < MAX_LOBES_DIFF && llabs(curPart->mLobeDepth - sParameters.mLobeDepth) < MAX_LOBEDEPTH_DIFF)))
		{
			// check to see if older branch's curve, curveBack, and curveV
			// are all close to the desired branch...
			F32 curveDiff = llabs(curPart->mCurve - sParameters.mCurve[level]);
			F32 curveBackDiff = llabs(curPart->mCurveBack - sParameters.mCurveBack[level]);
			F32 curveVDiff = llabs(curPart->mCurveV - sParameters.mCurveV[level]);
			if (curveDiff < MAX_CURVE_DIFF 
				&& curveVDiff < MAX_CURVE_V_DIFF
				&& (curveBackDiff == 0 || curveBackDiff < MAX_CURVEBACK_DIFF)
				)
			{
				// already have a branch that's close enough
				return i;
			}
		}
	}

	return -1;
}


void LLVOTreeNew::createPart(U8 level, F32 length, F32 radius, LLStrider<LLVector3> &vertices, LLStrider<LLVector3> &normals, 
							LLStrider<LLVector2> &tex_coords, U32 *indices, 
							U32 &curVertexIndex, U32 &curTexCoordIndex,
							U32 &curNormalIndex, U32 &curIndiceIndex)
{
	U8 i, j, k;

	// check for a similar branch...
	S32 selectedPart = findSimilarPart(level);
	if (selectedPart > -1) return;

	// if we didn't find the branch, make different versions of it
	// depending on how large curveV (curveVariance), make more branches...
	S32 numVariants = 1;
	numVariants += (S32)(sParameters.mCurveV[level]/CURVEV_DIVIDER);

	if (numVariants > MAX_VARS) numVariants = MAX_VARS;
	if (level == 0) numVariants = 1;	// just make one trunk

	llinfos << "TREE generating " << numVariants << " variants for curveV of " << sParameters.mCurveV[level] << " on level " << U32(level) << llendl;


	// if we've hit the max part limit, just quit...
	if (sNextPartIndex[mSpecies] == MAX_PARTS)
	{
		llinfos << "TREE SPECIES RAN OUT OF DRAWPOOL PART SPACE..." << llendl;
		return;
	}
	
	// put the first variant in its own mTreePart
	TreePart *part = &(LLVOTreeNew::sTreeParts[mSpecies][LLVOTreeNew::sNextPartIndex[mSpecies]++]);
	
	// set the tree part params from the basic tree params...
	part->mCurve = sParameters.mCurve[level];;
	part->mCurveRes = sParameters.mCurveRes[level];
	part->mCurveBack = sParameters.mCurveBack[level];
	part->mCurveV = sParameters.mCurveV[level];
	part->mLobeDepth = sParameters.mLobeDepth;
	part->mLobes = sParameters.mLobes;
	part->mLength = length;
	part->mRadius = radius;
	part->mLevel = level;
	part->mVertsPerSection = sParameters.mVertices[level];
	part->mNumVariants = 0;
	
	// enforce constraints on # of resolutions
	if (sParameters.mFlareRes > MAX_FLARE) sParameters.mFlareRes = MAX_FLARE;
	if (part->mCurveRes > MAX_RES) part->mCurveRes = MAX_RES;

	for (k = 0; k < numVariants; k++)
	{
		part->mNumVariants++;

		// set the part's vertices to the first vertex and first index of its parts
		part->mIndiceIndex[k] = curIndiceIndex;
		
		// how many total sections in the branch
		U8 numSections = (U8)(part->mCurveRes) + (level == 0 ? sParameters.mFlareRes : 0);
		
		U8 framesUsed = 0;
		U8 flareFramesUsed = 0;
		F32 stemZ;
		F32 startingZ = 0;
		
		// create curveRes different versions of the branch, with each
		// one having its origin base be a different flareRes or curveRes base
		
		U32 oldOffset = mRandOffset[level];
		
		for (j = 0; j < numSections; j++) 
		{
			mRandOffset[level] = oldOffset;
			if (j != 0) part->mOffsets[k][j - (level == 0 ? sParameters.mFlareRes : 0)] = curIndiceIndex;
			
			F32 sectionRadius;
			LLMatrix4 curFrame;
			U8 curSection = 0;
			
			// our position in the branch
			stemZ = startingZ;
			
			// Make branches from 
			// From 0th section...numSections
			//		1st section...numSections
			//		2nd section...numSections
			
			sectionRadius = CalculateSectionRadius(level, stemZ / part->mLength, part->mLength, part->mRadius);
			
			// create points that make up the bottom of the section...
			createSection(curFrame, part, sectionRadius, stemZ,
				vertices, tex_coords, indices, 
				curVertexIndex, curTexCoordIndex, curIndiceIndex, curSection++, j == 0 ? TRUE : FALSE);
			
			for (i = j; i < numSections; i++)
			{
				// do the curving...
				F32 angle;
				if (part->mCurveBack != 0)
				{		
					if (part->mCurveRes / (i + 1.0) < 2) 
						angle = part->mCurve / part->mCurveRes / 2.f;
					else
						angle = part->mCurveBack / part->mCurveRes / 2.f;
				}
				else
				{
					if (part->mLevel == 0)
					{
						if (i <= sParameters.mFlareRes) angle = (sParameters.mFlarePercentage*part->mCurve)/sParameters.mFlareRes;
						else angle = ((1.f - sParameters.mFlarePercentage)*part->mCurve)/part->mCurveRes;
					}
					else
						angle = part->mCurve / part->mCurveRes;
					
				}
				
				// add in variance just for kicks...
				angle += llfrand_signed(part->mCurveV/part->mCurveRes, mRandOffset[level + i]++);
				
				/* vertical attraction is based on height of branch which is unavailable information
				if (level > 1)
				angle += CalculateVerticalAttraction(level, curFrame);
				//*/
				
				angle *= DEG_TO_RAD;
				
				// translation...
				F32 zStep;
				zStep = CalcZStep(part, i);
				stemZ += zStep;
				
				// set the next startingZ as current stemZ if at first level
				if (i == j || (level == 0 && j == 0 && i == sParameters.mFlareRes - 1))
					startingZ = stemZ;
				
				LLQuaternion rotateQuat, oldRotateQuat, newRotateQuat;
				LLMatrix4 localFrame, transFrame;
				
				rotateQuat.setQuat(angle, 1.0f, 0.0f, 0.0f);
				oldRotateQuat.setQuat(curFrame);
				newRotateQuat = oldRotateQuat * rotateQuat;
				
				transFrame.translate(LLVector3(0, 0, zStep));
				transFrame.rotate(newRotateQuat);
				
				localFrame.rotate(newRotateQuat);
				localFrame.translate(curFrame.getTranslation());
				localFrame.translate(transFrame.getTranslation());
				
				curFrame = localFrame;
				
				// store flare frames seperately
				if (part->mLevel == 0 && i < sParameters.mFlareRes)
					mTrunkFlareFrames[flareFramesUsed++] = curFrame;
				else			
					part->mFrames[k][framesUsed++] = curFrame;
				
				// calc radius of section
				if (part->mLength == 0)
					sectionRadius = 0;
				else
					sectionRadius = CalculateSectionRadius(level, stemZ / part->mLength, part->mLength, part->mRadius);
				
				// create points that make up the section
				createSection(curFrame, part, sectionRadius, stemZ,
					vertices, tex_coords, indices,
					curVertexIndex, curTexCoordIndex, curIndiceIndex, curSection++, j == 0 ? TRUE : FALSE);
			}
			
			// only do one trunk w/ flaring.
			if (level == 0 && j == 0) j = sParameters.mFlareRes - 1;
		}
		
		// gen vertex normals now that we've created face normals for the first section...
		genVertexNormals(part, normals, numSections, curNormalIndex);
	}

}




void LLVOTreeNew::createSection(LLMatrix4 &frame, TreePart *part, F32 sectionRadius, F32 stemZ, 
							  LLStrider<LLVector3> &vertices, LLStrider<LLVector2> &tex_coords, U32 *indices, 
							  U32 &curVertexIndex, U32 &curTexCoordIndex, U32 &curIndiceIndex, U8 curSection, BOOL firstBranch)
{

    F32 angle;							// Angle holds the angle in radians
                                        // between two points in the section.

    LLVector3 localPoint;               // LocalPoint is used to hold the points
                                        // that are being created and added to
                                        // the points list of the section (in
                                        // local coordinates)

    LLVector3 globalPoint;              // GlobalPoint is used to hold the points
                                        // that are being created and added to
                                        // the points list of the section (in
                                        // global coordinates)

    F32 lobedSectionRadius;				// The sectionradius rescaled using
				                        // the lobing parameters

	F32 percentX, percentY;				// The texture coords



	U8 numVerts = part->mVertsPerSection;

	// prevent empty triangles
	if (sectionRadius < 0.0001) {
		sectionRadius = 0.0001f;
	}

	// Y texture coordinate
	// stemZ is current position within mLength...
	percentY = stemZ;
	//percentY = stemZ/part->mLength;

	angle = (2 * F_PI) / numVerts;
	for (U8 i = 0; i <= numVerts; i++)
	{
		// last vertex is same as first one...
		if (i == numVerts)
		{
			// make first vertice the last vertice (wrap around so that our texture coords will be ok...
			globalPoint = vertices[curVertexIndex - numVerts]; 
		}
		else
		{
			// lobed = 1.0 + lobeDepth * sin(lobes * angle)
			lobedSectionRadius = sectionRadius * (1.f + sParameters.mLobeDepth * sin(sParameters.mLobes * (i + 1) * angle));

			localPoint.mV[0] = cos((i + 1.f) * angle) * lobedSectionRadius;
			localPoint.mV[1] = sin((i + 1.f) * angle) * lobedSectionRadius;
			localPoint.mV[2] = 0;

			globalPoint = localPoint * frame;
		}

		if (curVertexIndex > NUM_INIT_VERTS) {
			llinfos << "TREE ERROR NO MORE VERTS" << llendl;
			return;
		}

		// add to instanced data...
		vertices[curVertexIndex++] = globalPoint;
		mTotalVerts++;

		/*
		// update the max x, y, and z of tree 
		if (globalPoint->mV[0] > mTree->mMaxX) { mTree->mMaxX = globalPoint->mV[0]; }
		if (globalPoint->mV[1] > mTree->mMaxY) { mTree->mMaxY = globalPoint->mV[1]; }
		if (globalPoint->mV[2] > mTree->mMaxZ) { mTree->mMaxZ = globalPoint->mV[2]; }
		*/

		// TEXTURE COORDS
		percentX = WIDTH_OF_BARK * (i/F32(numVerts));// * (2.0*F_PI*sectionRadius);
		//percentX =  * (i/F32(numVerts));
		tex_coords[curTexCoordIndex++] = LLVector2(percentX, percentY);
	}

	// gen face normals and do texcoords...
	if (curSection != 0)
		genIndicesAndFaceNormalsForLastSection(part, numVerts, vertices, curVertexIndex, indices, curIndiceIndex, firstBranch);
}


// generate face normals for the last two cross sections in sectionlist
void LLVOTreeNew::genIndicesAndFaceNormalsForLastSection(TreePart *part, U8 numVerts, LLStrider<LLVector3> &vertices, U32 curVertexIndex, U32 *indices, U32 &curIndiceIndex, BOOL firstBranch)
{

	LLVector3 v1, v2;
	LLVector3 vCross;
	LLVector3 a, b, c, d;

	// offsets into the vertex array
	U32 upperOffset = curVertexIndex - (numVerts + 1);
	U32 lowerOffset = upperOffset - (numVerts + 1);

	// generate the normals for the triangles from the quads
		// quad is defined by:
		// upper[i].....upper[i+1]
		//    .             .
		//    .             .
		//    .             .
		// lower[i].....lower[i+1]

		// 10.......11
		// .  .     .
		// .    .   .
		// .      . .
		// 00 . . . 01

		// b . . . d
		// . .     .
		// .   .   .
		// .     . .
		// a . . . c

	for (U8 j = 0; j < numVerts; j++ ) {

		U8 nextVert = j + 1;

		// do face normals for first version only
		if (firstBranch)
		{
			// the points of the quad
			a = vertices[lowerOffset];				// 00
			b = vertices[upperOffset];				// 10
			c = vertices[lowerOffset + nextVert];	// 01
			d = vertices[upperOffset + nextVert];	// 11

			// 1st triangle
			v1 = c - b;
			v2 = a - b;
			vCross = v1 % v2;
			vCross.normVec();
			part->mFaceNormals.put(vCross);

			// 2nd triangle
			v1 = c - b;
			v2 = d - b;
			vCross = v1 % v2;
			vCross.normVec();
			part->mFaceNormals.put(vCross);
		}

		if (curIndiceIndex + 6 > NUM_INIT_INDICES)
		{
			llinfos << "TREE ERROR NO MORE INDICES" << llendl;
			return;
		}

		indices[curIndiceIndex++] = lowerOffset + j;			// 00
		indices[curIndiceIndex++] = lowerOffset + nextVert;		// 01
		indices[curIndiceIndex++] = upperOffset + j;			// 10
		if (firstBranch) { part->mNumTris++; }

		indices[curIndiceIndex++] = lowerOffset + nextVert;		// 01
		indices[curIndiceIndex++] = upperOffset + nextVert;		// 11
		indices[curIndiceIndex++] = upperOffset + j;			// 10                                                                                                                                                                                                                                                                                                                                                                                                       
		if (firstBranch) { part->mNumTris++; }

		mTotalIndices += 6;
	//*/
	}
}




void LLVOTreeNew::genVertexNormals(TreePart *part, LLStrider<LLVector3> &normals, U8 numSections, U32 curNormalIndex)
{
	LLVector3 vNormal;

	U8 numVerts = part->mVertsPerSection;
	U16 numFaces = numVerts * 2;

	U32 curSectionFaceOffset, lowerSectionFaceOffset;
				
	U8 i;
	U8 j;

	// for each section...
	for (i = 0; i < numSections; i++) {

		U32 numFacesStored = part->mFaceNormals.count();
		// index into face normals for 0-ith face of section i...
		curSectionFaceOffset = numFacesStored - (numSections - 1 - i)*numFaces;
		lowerSectionFaceOffset = curSectionFaceOffset - numFaces;

		// for each vertex...
		for (j = 0; j < numVerts; j++) {

			// if lowest level or highest level...
			if (i == 0) {
				// bottom 3 adjacent tris
				vNormal += part->mFaceNormals[curSectionFaceOffset + (numFaces - j*2 - 2) % numFaces];
				vNormal += part->mFaceNormals[curSectionFaceOffset + (numFaces - j*2 - 1) % numFaces];
				vNormal += part->mFaceNormals[curSectionFaceOffset + (numFaces - j*2) % numFaces];
				vNormal /= 3.0;
			} else if (i == numSections - 1) {
				// top 3 adj tris
				vNormal += part->mFaceNormals[lowerSectionFaceOffset + (numFaces - j*2 - 1) % numFaces];
				vNormal += part->mFaceNormals[lowerSectionFaceOffset + (numFaces - j*2) % numFaces];
				vNormal += part->mFaceNormals[lowerSectionFaceOffset + (numFaces - j*2 + 1) % numFaces];
				vNormal /= 3.0;
			}
			else
			{
				// otherwise avg the normals from the 6 adjacent tris
				// avg the 6 normal vectors that are adjacent to it...
				// vertex 0 is surrounded by...
				//		lower: 0, 1  .... n
				//		upper: n-1, n .... 0

				vNormal += part->mFaceNormals[curSectionFaceOffset + (numFaces - j*2 - 2) % numFaces];
				vNormal += part->mFaceNormals[curSectionFaceOffset + (numFaces - j*2 - 1) % numFaces];
				vNormal += part->mFaceNormals[curSectionFaceOffset + (numFaces - j*2) % numFaces];

				vNormal += part->mFaceNormals[lowerSectionFaceOffset + (numFaces - j*2 - 1) % numFaces];
				vNormal += part->mFaceNormals[lowerSectionFaceOffset + (numFaces - j*2) % numFaces];
				vNormal += part->mFaceNormals[lowerSectionFaceOffset + (numFaces - j*2 + 1) % numFaces];

				vNormal /= 6.0;
			}
			normals[curNormalIndex] = vNormal;
			curNormalIndex++;
		}
		// get first normal and use for the repeated last vertex...
		normals[curNormalIndex] = normals[curNormalIndex - numVerts];
		curNormalIndex++;
	}


	// there are mCurveRes - 1 different starting points...
	U8 offset = 0;
	U8 curveres8 = (U8)part->mCurveRes;
	for (i = 0; i < curveres8 - 1; i++)
	{
		U8 numSectionsInBranch = curveres8 - i;
		U8 origOffset = i * numVerts;

		for (j = 0; j <= numSectionsInBranch * numVerts; j++)
			normals[curNormalIndex + offset + j] = normals[curNormalIndex + origOffset + j];

		offset += numSectionsInBranch * numVerts;
	}

	
}



//{ Pre: 0 <= y <= 1
//  Ret: The radius of the stem at the (normalized) y position along the stem.
//       See for the exact calculations the paper "Creation and Rendering of
//       Realistic Trees", by Jason Weber and Joseph Penn. }
	   

F32 LLVOTreeNew::CalculateSectionRadius(U8 level, F32 y, F32 stemLength, F32 stemRadius)
{
	F32 y2, y3;

	F32 depth;		// Scaling factor used for periodic tapering
	F32 taperY;		// Tapered radius along at the (normalized) y
                    // position along the stem
	F32 unitTaper;	// UnitTaper is used to determine the radius of
                    // the stem along a specified (normalized)
                    // position along the stem.

	F32 radius;		// radius returned	

	//    { Calculate UnitTaper, a variable used to determine the radius of the
    //	stem along a specified (normalized) position Z along the stem }

	unitTaper = 0;

	F32 curTaper = sParameters.mTaper[level];	// cur taper for this level

	// 0 <= nTaper < 1
	//	unitTaper = nTaper
	// 1 <= nTaper < 2
	//	unitTaper = 2 - nTaper
	// 2 <= nTaper < 3
	//	unitTaper = 0

	if ( (curTaper >= 0) && (curTaper < 1) )
		unitTaper = curTaper;
	else if ( (curTaper >= 1) && (curTaper < 2) )
		unitTaper = 2.f - curTaper;
	else	// should be 2 <= curTaper < 3
		unitTaper = 0;


	// y is ratio of stemY/length...sometimes [something.mumble] / [something.mumble] != 1 when it should...
	if (y > 1.0)
		y = 1.0;

	// taperY = radius[stem] * (1 - unitTaper * y)
	taperY = stemRadius * (1.f - unitTaper * y);

	// 0 <= curTaper <= 1
	//	radius = taperY;

	if ( (curTaper >= 0) && (curTaper < 1) )
	{
		radius = taperY;
	}
	else
	{
		// { (nTaper[ALevel] >= 1) and (nTaper[ALevel] <= 3) }

		// initialize y2
		y2 = (1 - y) * stemLength;

		// initialize depth
			
		if ( (curTaper < 2) || (y2 < taperY) )
			depth = 1;
		else
			depth = curTaper - 2;

		// initialize y3
		if ( curTaper < 2) 
			y3 = y2;
		else
			y3 = fabs(y2 - 2 * taperY * floor(y2 / (2 * taperY) + 0.5f));
		
		// return the radius
		if ( (curTaper < 2) && (y3 >= taperY) )
			radius = taperY;
		else
			radius = (1 - depth) * taperY + depth * sqrt(pow(taperY, 2) - pow(y3 - taperY, 2));
	}

	// calculate flaring
	if (level == 0)
	{
		y2 = 1 - 8 * y;
		if (y2 < 0) 
			y2 = 0;

		// flare = Flare * (100^y - 1) / 100 + 1
		radius = radius * (sParameters.mFlare * (pow((F32)100, y2) - 1) / 100 + 1);
	}

	return radius;
}

/*
F32 LLVOTreeNew::CalculateVerticalAttraction(U8 level, LLMatrix4 &sectionFrame)
{
	LLVector3 transformY, transformZ;
	LLVector3 unitY(0.0, 1.0, 0.0);
	LLVector3 unitZ(0.0, 0.0, 1.0);
	F32 declination, orientation;
	
	transformY = unitY * sectionFrame;
	transformZ = unitZ * sectionFrame;

	declination = acos(transformZ.mV[1]);
	orientation = acos(transformY.mV[1]);

	return (sParameters.mAttractionUp * declination * cos(orientation)) / sParameters.mCurveRes[level];
}
//*/


void LLVOTreeNew::drawTree(LLDrawPool &draw_pool)
{
	U8 i, j;

	// seed the drawtree thing with the object's uuid to make it original but predictable...
	gLindenLabRandomNumber.seed(0);

	// reset the rand offsets
	for (i = 0; i < MAX_LEVELS; i++) mRandOffset[i] = 0;

	F32 trunkLength = sParameters.mLength[0] * sParameters.mScale;
	//F32 trunkLength = sParameters.mLength[0] + llfrand_signed(sParameters.mLengthV[0])) * 
	//	(sParameters.mScale + llfrand_signed(sParameters.mScaleV));
	F32 trunkRad = trunkLength * sParameters.mRatio * sParameters.mScale0;

//*	reset usage data on which part is grabbed
	mNumTrisDrawn = 0;
	for (i = 0; i < MAX_PARTS; i++)
		for (j = 0; j < MAX_VARS; j++)
			sTreePartsUsed[mSpecies][i][j] = 0;
	
	// reset segsplit error stuff...
	for (j = 0; j < 3; j++)
		mSegSplitsError[j] = 0;

	//llinfos << "\nTREE Starting" << llendl;
	drawTree(draw_pool, LLMatrix4(), 0, 0, trunkLength, 0, trunkRad, 0, 0, 0, 0);
	//llinfos << "\nTREE Ending" << llendl;
	
	/*
	// print part usage info:
	for (i = 0; i < MAX_PARTS; i++)
	{
		llinfos << "TREE part: " << U32(i) << ": " << llendl;
		for (j = 0; j < MAX_VARS; j++)
			llinfos << "\t variant " << U32(j) << ": [" << U32(sTreePartsUsed[mSpecies][i][j]) << "], " << llendl;
		llinfos << "" << llendl;
	}
	//*/

/*
	glTranslatef(0.0, 0.0, 5.0);
	for (i = 0; i < sNextPartIndex[mSpecies]; i++)
	{
		TreePart *selectedPart = &(sTreeParts[mSpecies][i]);
		glTranslatef(2.0, 0.0, 0.0);

		// draw a variant...
		for (U8 k = 0; k < selectedPart->mNumVariants; k++)
		{
			// start at the indice index
			U32 prevOffset = 0;//selectedPart->mIndiceIndex[k];

			
			if (selectedPart->mLevel == 0 && k == 0)
			{
				// draw the original one with flare...
				glTranslatef(1.0, 0.0, 0.0);
				glPushMatrix();
				glDrawElements(GL_TRIANGLES, selectedPart->mNumTris*3, GL_UNSIGNED_INT, draw_pool.getRawIndices() + selectedPart->mIndiceIndex[k]);
				glPopMatrix();
				prevOffset += selectedPart->mNumTris*3;
			}

			// draw each of the curveRes iterations...
			for (j = 0; j < selectedPart->mCurveRes; j++)
			{
				U32 numTris = (selectedPart->mCurveRes - j) * selectedPart->mVertsPerSection * 2;
				glTranslatef(0.0, 1.0, 0.0);
				glPushMatrix();
				glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_INT, draw_pool.getRawIndices() + selectedPart->mIndiceIndex[k] + prevOffset);
				glPopMatrix();
				prevOffset += numTris*3;
			}
		}
	}
//*/
}

void LLVOTreeNew::drawTree(LLDrawPool &draw_pool, const LLMatrix4 &frame, U8 level, F32 offsetChild, 
						   F32 curLength, F32 parentLength, F32 curRadius, F32 parentRadius,
						   U8 part, U8 variant, U8 startSection)
{
	U8 i, j, k;

	// if startSection != 0, we're doing a segsplit.

	// find the tree part most similar (if we're not doing a segsplit)
	if (startSection == 0)
	{
		S32 similarNum = findSimilarPart(level);
		if (similarNum < 0) part = level;
		else part = similarNum;
	}

	TreePart *selectedPart;
	selectedPart = &(sTreeParts[mSpecies][part]);
	
	// pick a variant if we're not in a segsplit (and therefore already have one chosen)
	//if (startSection == 0) variant = llrand_unsigned(selectedPart->mNumVariants + 1, mRandOffset[level]++);
	//if (variant == selectedPart->mNumVariants) variant--;
	// manual override
	variant = 0;

	sTreePartsUsed[mSpecies][part][variant]++;

	//llinfos << "TREE part desired: " << U32(level) << ", " << sParameters.mLobeDepth << ", " << curAngleInt << llendl;
	//llinfos << "TREE part found: " << U32(selectedPart->mLevel) << ", " << selectedPart->mLobeDepth << ", " << selectedPart->mAngle << llendl;

	// determine the scalers so that we meet the desired length/radius...
	F32 lengthMultiplier = curLength / selectedPart->mLength;
	F32 radiusMultiplier = curRadius / selectedPart->mRadius;
	LLMatrix4 scaleMatrix;
	scaleMatrix.mMatrix[0][0] = radiusMultiplier;
	scaleMatrix.mMatrix[1][1] = radiusMultiplier;
	scaleMatrix.mMatrix[2][2] = lengthMultiplier;

	// the global rotation this branch will take
	LLQuaternion frameQuat;
	frameQuat.setQuat(frame);
	LLMatrix4 scaledFrame = frame;
	scaledFrame *= scaleMatrix;
	
	// start from the previous startSection of the curveRes...try to go till we're at the end of the branch
	// i is the current section we're looking at for segment splits
	
	F32 otherRotateAngle = llfrand_signed(360.0f, mRandOffset[level]++);

	U8 curSection = startSection;
	for (i = startSection; i < selectedPart->mCurveRes - 1; i++)
	{
		curSection = i + 1;

		S32 segSplitsEffective = 0;

		// seg split code taken from confusing paragraph of Weber + Penn
		F32 segSplit = llfrand_unsigned(sParameters.mSegSplits[level], mRandOffset[level]++);
		segSplitsEffective = llround(segSplit + mSegSplitsError[level]);
		mSegSplitsError[level] += segSplit - lltrunc(segSplit);
		mSegSplitsError[level] -= (segSplitsEffective - segSplit);

		// if we've got a seg split, figure out the right rotated frames and send them off...
		if (segSplitsEffective > 0)
		{
			// figure out the declination for this height...
			LLVector3 unitZ(0.0f, 0.0f, 1.0f);
			LLVector3 transformZ = unitZ * frame;
			F32 declination = RAD_TO_DEG*acos(transformZ.mV[1]);

			F32 splitAngle = (sParameters.mSplitAngle[level] + llfrand_unsigned(sParameters.mSplitAngleV[level], mRandOffset[level]++)) - declination;
			if (splitAngle < 0) 
				splitAngle = 0;
			//splitAngle = sParameters.mDownAngle[2];//45.0f;
			
			// do each seg split...
			for (j = 0; j < segSplitsEffective; j++)
			{
				otherRotateAngle += llround(20.0f + 0.75f*(30.0f + fabs(declination - 90.0f)) * pow(fabs((F32)llrand_signed(1, mRandOffset[level]++)), 2.0f));

				//otherRotateAngle += sParameters.mDownAngle[1];//((j + 1) * 360.0)/(segSplitsEffective + 1);
				
				U32 frameOffset = 0;
				if (startSection != 0)
				{
					// offset into the branch that begins with that startsection...
					if (level == 0) frameOffset += (U32)selectedPart->mCurveRes - 1;
					// i is our start section ...
					for (k = 0; k < i; k++)	
						frameOffset += ((U32)selectedPart->mCurveRes - k);

					frameOffset--;
				}

				LLMatrix4 segFrame;
				segFrame = selectedPart->mFrames[variant][frameOffset + curSection - 1];
				segFrame *= scaleMatrix;

				LLVector3 segFrameTrans = segFrame.getTranslation();
				segFrameTrans.rotVec(frameQuat);

				LLQuaternion splitQuat, rotateQuat;
				splitQuat.setQuat(splitAngle*DEG_TO_RAD, 1.0f, 0.0f, 0.0f);
				rotateQuat.setQuat(otherRotateAngle*DEG_TO_RAD, 0.0f, 0.0f, 1.0f);

				LLMatrix4 splitFrame;
				//splitFrame.rotate(splitQuat * rotateQuat * frameQuat); ??
				splitFrame.rotate(frameQuat * splitQuat * rotateQuat);
				splitFrame.translate(segFrameTrans);
				splitFrame.translate(frame.getTranslation());

				// recurse from the current endsection as start section and with the new frame as base
				drawTree(draw_pool, splitFrame, level, offsetChild, 
				   curLength, parentLength, curRadius, parentRadius,
				   part, variant, curSection);
			}
		}
	}


	U32 startIndexOffset = 0;

	if (startSection != 0)
		startIndexOffset = selectedPart->mOffsets[variant][startSection];

	U32 numTris;
	if (level == 0 && startSection == 0)
		numTris = selectedPart->mNumTris;
	else
		numTris = 2 * selectedPart->mVertsPerSection * ((U8)selectedPart->mCurveRes - startSection);

	glPushMatrix();
		glMultMatrixf((float *)frame.mMatrix);
		glScalef(radiusMultiplier, radiusMultiplier, lengthMultiplier);
	
		//glDrawElements(GL_TRIANGLES, selectedPart->mNumTris*3, GL_UNSIGNED_INT, draw_pool.getRawIndices() + selectedPart->mIndiceIndex + 0);
		glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_INT, draw_pool.getRawIndices() + selectedPart->mIndiceIndex[variant] + startIndexOffset);
	glPopMatrix();

	mNumTrisDrawn += numTris;

	// figure out how many in next level of branches || leaves
	U8 total = 0;
	F32 baseLength = 0;

	if (level < sParameters.mLevels) 
	{
		if (level == 0)
		{
			baseLength = sParameters.mBaseSize * (sParameters.mScale + llfrand_signed(sParameters.mScaleV, mRandOffset[level]++));
			total = llround( (1.f - sParameters.mBaseSize) * sParameters.mBranches[level]);
		}
		else if (level == 1)
		{
			F32 maxLength = sParameters.mLength[level] + llfrand_signed(sParameters.mLengthV[level], mRandOffset[level]++);
			total = llround(sParameters.mBranches[level] * (0.2f + 0.8f * ( (curLength/parentLength)/maxLength ) ) );
		}
		else
		{
			total = (llround(sParameters.mBranches[level] * (1.0f - 0.5f * (offsetChild/parentLength) )));
		}
	}
	else if (sParameters.mLevels == level)
	{
		// determine leaves
		// see paper for more on:
		// leaves_per_branch = Leaves * 
		//		ShapeRatio ( 4 (tapered), offset[child]/length[parent] * quality)
		
		//total = (sParameters.mLeaves * sParameters.ShapeRatio(SR_CONICAL, offsetChild / curLength) * sParameters.mLeafQuality);
		total = sParameters.mLeaves;
		
	}

	F32 fracPos, offsetPos, localPos;
	F32 downAngle, rotateAngle;
	U8 currentFrameSegment;		// index of matrix transform of the closest cross section to new branch
	LLMatrix4 localFrame, curPartFrame, nextPartFrame;

	// init stem rotate angle...
	rotateAngle = llfrand_unsigned(360.0, mRandOffset[level]++);
		
	// grow the leaf or branch...
	for (i = 0; i < total; i++)
	{
		// note: curLength is the 'parentLength' for all of these substems
		localFrame.identity();

		// find the fractional, offset and then local pos
		if (level == 0)
			fracPos = sParameters.mBaseSize + (i) * (1.0f - sParameters.mBaseSize) / (total);
		else
			fracPos = (i) * (1.0f / (total));

		// offset into branch
		offsetPos = fracPos*curLength;

		U32 frameOffset = 0;
		if (startSection != 0) 
		{
			if (level == 0) frameOffset = (U32)selectedPart->mCurveRes;

			for (k = 0; k < startSection; k++)
				frameOffset += ((U32)selectedPart->mCurveRes - k);
		}

		F32 curSegAt;
		if (level == 0 && startSection == 0)
		{
			F32 effectiveFlarePercentage = sParameters.mFlarePercentage + (1.0f - sParameters.mFlarePercentage)/selectedPart->mCurveRes;

			if (fracPos < effectiveFlarePercentage)
			{
				if (fracPos > sParameters.mFlarePercentage)
					currentFrameSegment = sParameters.mFlareRes - 1;
				else
					currentFrameSegment = lltrunc( (fracPos/sParameters.mFlarePercentage) / (1.0/sParameters.mFlareRes) );
				
				if (currentFrameSegment == 0) 
				{ 
					curPartFrame.identity(); 
					nextPartFrame = mTrunkFlareFrames[0]; 
				}
				else 
				{ 
					curPartFrame = mTrunkFlareFrames[currentFrameSegment - 1]; 
					if (currentFrameSegment == sParameters.mFlareRes - 1) nextPartFrame = selectedPart->mFrames[variant][0]; 
					else nextPartFrame = mTrunkFlareFrames[currentFrameSegment]; 
				}
				
				curSegAt = currentFrameSegment * (curLength * sParameters.mFlarePercentage)/sParameters.mFlareRes;
				localPos = offsetPos - curSegAt;
			}
			else
			{
				// want to go from frameSegment 0 to (1-CurveRes)...don't want to have frame at tip
				currentFrameSegment = lltrunc( ( (fracPos - effectiveFlarePercentage)/(1.0 - effectiveFlarePercentage) ) / (1.0 / (selectedPart->mCurveRes - 1)) );
				curSegAt = (sParameters.mFlarePercentage * curLength) + (currentFrameSegment + 1.0f) * (  (curLength*(1.0f - sParameters.mFlarePercentage)/(selectedPart->mCurveRes)) );
				localPos = offsetPos - curSegAt;
				curPartFrame = selectedPart->mFrames[variant][currentFrameSegment];
				nextPartFrame = selectedPart->mFrames[variant][currentFrameSegment + 1];
			}
			//llinfos << "TREE startsection: " << U32(startSection) << " currentSeg: " << U32(currentFrameSegment) << ", is at: " << curSegAt << ", fracpos: " << fracPos << ", offset: " << offsetPos << ", localpos: " << localPos << ", final: " << curSegAt + localPos << llendl;			
		}
		else if (level == 0 && startSection != 0)
		{
			// if level is zero, don't deal with flared branch positions
			if (fracPos < sParameters.mFlarePercentage) continue;

			// take FlarePercentage out of fracPos 
			fracPos = (fracPos - sParameters.mFlarePercentage)/(1 - sParameters.mFlarePercentage);

			currentFrameSegment = lltrunc(fracPos / (1.0 / selectedPart->mCurveRes));
			
			// don't do branches below our split starting point
			if (currentFrameSegment < startSection) continue;

			curSegAt = (sParameters.mFlarePercentage * curLength) + currentFrameSegment*((curLength - sParameters.mFlarePercentage*curLength)/(selectedPart->mCurveRes));
			localPos = offsetPos - curSegAt;
			currentFrameSegment -= startSection;

			if (currentFrameSegment == 0) 
			{ 
				curPartFrame.identity(); 
				nextPartFrame = selectedPart->mFrames[variant][frameOffset]; 
			}
			else 
			{
				curPartFrame = selectedPart->mFrames[variant][frameOffset + currentFrameSegment - 1];
				nextPartFrame = selectedPart->mFrames[variant][frameOffset + currentFrameSegment];
			}
		}
		else
		{
			currentFrameSegment = lltrunc(fracPos / (1.0 / selectedPart->mCurveRes));

			// don't do branches below our split starting point
			if (currentFrameSegment < startSection) continue;

			curSegAt = (currentFrameSegment) * (curLength/(selectedPart->mCurveRes));
			localPos = offsetPos - curSegAt;
			currentFrameSegment -= startSection;

			if (currentFrameSegment == 0) 
			{ 
				curPartFrame.identity();
				nextPartFrame = selectedPart->mFrames[variant][frameOffset];
			}
			else 
			{
				curPartFrame = selectedPart->mFrames[variant][frameOffset + currentFrameSegment - 1];
				nextPartFrame = selectedPart->mFrames[variant][frameOffset + currentFrameSegment];
			}
		}

		if (sParameters.mDownAngle[level] >= 0) 
			downAngle = sParameters.mDownAngle[level] + llfrand_signed(sParameters.mDownAngleV[level], mRandOffset[level]++);
		else 
			downAngle = sParameters.mDownAngle[level] + sParameters.mDownAngleV[level] *
				(1 - 2.0f * sParameters.ShapeRatio(SR_CONICAL, (curLength - offsetPos) / (curLength - baseLength)));

		if (sParameters.mRotate[level] >= 0)
			rotateAngle += (sParameters.mRotate[level] + llfrand_signed(sParameters.mRotateV[level], mRandOffset[level]++));
		else
			rotateAngle += (180.0f + sParameters.mRotate[level] + llfrand_signed(sParameters.mRotateV[level], mRandOffset[level]++));

		if (sParameters.mLevels == level)
		{
			downAngle = 0.0; 
			rotateAngle = 0.0;
		}

// manual overrides
//		rotateAngle = sParameters.mRotate[0];
//		downAngle = sParameters.mDownAngle[0];

		LLQuaternion downQuat, rotateQuat, nextQuat, curQuat;
		downQuat.setQuat(DEG_TO_RAD*downAngle, 1.0, 0.0, 0.0);
		rotateQuat.setQuat(DEG_TO_RAD*rotateAngle, 0.0, 0.0, 1.0);
		nextQuat.setQuat(nextPartFrame);
		curQuat.setQuat(curPartFrame);

		localFrame.identity();

		// take the home frame, and rotate by the global frame, translate to the global start
		curPartFrame.rotate(frameQuat);

		// rotate to the final angle and translate to the bottom of the branch

		localFrame.rotate(frameQuat * nextQuat * downQuat * rotateQuat);
		// scale our parent frame to put stuff in the right location
		curPartFrame *= scaleMatrix;
		localFrame.translate(curPartFrame.getTranslation());
		//localFrame.translate(scaledFrame.getTranslation());
		localFrame.translate(frame.getTranslation());

		// offset the branch by a bit and then rotate using current position, add the resulting translation
		LLVector3 translateUp(0.0, 0.0, localPos);
		translateUp.rotVec(nextQuat * frameQuat);
		//translateUp.rotVec(nextQuat);
		localFrame.translate(translateUp);

		
		// BRANCHES...
		if (level < sParameters.mLevels)
		{
			F32 subStemLength, maxChildLength;
			F32 localRadius, subStemRadius;

			// determine a maximum length for this stem's children...
			maxChildLength = sParameters.mLength[level+1] + llfrand_signed(sParameters.mLengthV[level+1], mRandOffset[level]++);

			// length of child branch
			if (level == 0)
				subStemLength = curLength * maxChildLength * 
					sParameters.ShapeRatio(sParameters.mShape, (curLength - offsetPos) / (curLength - baseLength));
			else
				subStemLength = maxChildLength * (curLength - 0.6f * offsetPos);

			// radius of child branch
			if (curLength == 0) 
				subStemRadius = 0;
			else 
				subStemRadius = selectedPart->mRadius * pow((subStemLength / curLength), sParameters.mRatioPower);

			// don't have a radius that is larger than parent's
			localRadius = CalculateSectionRadius(level, fracPos, selectedPart->mLength, selectedPart->mRadius);
			localRadius *= radiusMultiplier;

			if ( (subStemRadius > localRadius) || (subStemRadius == 0) )
				subStemRadius = localRadius;

			if (subStemLength != 0)
				drawTree(draw_pool, localFrame, level + 1, offsetPos, subStemLength, curLength, subStemRadius, curRadius, 0, 0, 0);

		}
		// LEAVES
		else if (sParameters.mLevels == level)
		{
			// for now just draw leaves stupidly...

			// draw square leaf (two tris) with the texture on it (then do it again for the other side)
			mNumTrisDrawn += 4;
//*
			F32 leafXscale = 1.0f;
			F32 leafYscale = 1.0f;
			
			glPushMatrix();
			//LLVector3 position = localFrame.getTranslation();

			//glTranslatef(position.mV[0], position.mV[1], position.mV[2]);
			glMultMatrixf((float *)localFrame.mMatrix);
			
			glBegin(GL_QUADS);
			
			//*
			// 00
			glTexCoord2f(0.5f, 0.5f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			// 10
			glTexCoord2f(1.0f, 0.5f);
			glVertex3f(0.0f, 0.0f, leafXscale);
			// 11
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(0.0f, leafYscale, leafXscale);
			// 01
			glTexCoord2f(0.5f, 1.0f);
			glVertex3f(0.0f, leafYscale, 0.0f);
			//*/
			
			// 10
			glTexCoord2f(1.0f, 0.5f);
			glVertex3f(0.0f, 0.0f, leafXscale);
			// 00
			glTexCoord2f(0.5f, 0.5f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			// 01
			glTexCoord2f(0.5f, 1.0f);
			glVertex3f(0.0f, leafYscale, 0.0f);
			// 11
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(0.0f, leafYscale, leafXscale);

			glEnd();
			glPopMatrix();
//*/
		}
	}

}




