/** 
 * @file llhudconnector.cpp
 * @brief LLHUDConnector class implementation
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llhudconnector.h"
#include "llviewercamera.h"
#include "llviewerobject.h"

#include "llgl.h"
#include "llimagegl.h"
#include "llsphere.h"
#include "llhudrender.h"
#include "llfontgl.h"
#include "llglheaders.h"

LLHUDConnector::LLHUDConnector(const U8 type) :
			LLHUDObject(type),
			mZCompare(TRUE)
{
	mColor = LLColor4(1.f, 1.f, 1.f, 1.f);
	mFirstColor = mColor;
	mSecondColor = mColor;
	mDoFade = TRUE;
	mFadeDistance = 40.f;
	mFadeRange = 10.f;
	mDrawFirst = mDrawSecond = TRUE;

	mLabel = "Foo!";
}

LLHUDConnector::~LLHUDConnector()
{
}

void LLHUDConnector::setLabel(const LLString &label)
{
	mLabel = label;
}

void LLHUDConnector::setTargets(LLViewerObject *first_object, LLViewerObject *second_object)
{
	setSourceObject(first_object);
	setTargetObject(second_object);
}

void LLHUDConnector::render()
{
	if (mSourceObject.isNull() || mTargetObject.isNull())
	{
		mSourceObject = NULL;
		mTargetObject = NULL;
		return;
	}

	if (mSourceObject->isDead() || mTargetObject->isDead())
	{
		mSourceObject = NULL;
		mTargetObject = NULL;
		return;
	}

	LLVector3 first_pos_agent = mSourceObject->getPositionAgent();
	LLVector3 second_pos_agent = mTargetObject->getPositionAgent();

	LLVector3 center_agent = 0.5f*(first_pos_agent + second_pos_agent);
	F32 dist = (center_agent - gCamera->getOrigin()).magVec();
	F32 alpha_factor = 1.f;

	if (mDoFade)
	{
		if (dist > mFadeDistance)
		{
			alpha_factor = llmax(0.f, 1.f - (dist - mFadeDistance)/mFadeRange);
		}
	}

	if (alpha_factor < 0.01f)
	{
		return;
	}

	LLGLSPipelineAlpha gls_pipeline_alpha;
	LLImageGL::unbindTexture(0, GL_TEXTURE_2D);

	LLVector3 dir_vec = first_pos_agent - second_pos_agent;
	dir_vec.normVec();
	dir_vec *= 0.05f;

	LLVector3 first_line_pos = first_pos_agent - dir_vec;
	LLVector3 second_line_pos = second_pos_agent + dir_vec;

	LLColor4 color;

	// Spheres on ends of connectors
	if (mDrawFirst)
	{
		color = mFirstColor;
		color.mV[3] *= alpha_factor;
		glColor4fv(color.mV);
		glPushMatrix();
		glTranslatef(first_pos_agent.mV[0], first_pos_agent.mV[1], first_pos_agent.mV[2]);
		glScalef(0.1f, 0.1f, 0.1f);
		gSphere.render();
		glPopMatrix();
	}

	if (mDrawSecond)
	{
		color = mSecondColor;
		color.mV[3] *= alpha_factor;
		glColor4fv(color.mV);
		glPushMatrix();
		glTranslatef(second_pos_agent.mV[0], second_pos_agent.mV[1], second_pos_agent.mV[2]);
		glScalef(0.1f, 0.1f, 0.1f);
		gSphere.render();
		glPopMatrix();
	}

	color = mColor;
	color.mV[3] *= alpha_factor;
	glColor4fv(color.mV);
	glBegin(GL_LINES);
	glVertex3fv(first_line_pos.mV);
	glVertex3fv(second_line_pos.mV);
	glEnd();

	{
		LLGLDepthTest gls_depth(GL_FALSE);
		// Spheres on ends of connectors
		if (mDrawFirst)
		{
			color = mFirstColor;
			color.mV[3] *= 0.25f*alpha_factor;
			glColor4fv(color.mV);
			glPushMatrix();
			glTranslatef(first_pos_agent.mV[0], first_pos_agent.mV[1], first_pos_agent.mV[2]);
			glScalef(0.1f, 0.1f, 0.1f);
			gSphere.render();
			glPopMatrix();
		}
		if (mDrawSecond)
		{
			color = mSecondColor;
			color.mV[3] *= 0.25f*alpha_factor;
			glColor4fv(color.mV);
			glPushMatrix();
			glTranslatef(second_pos_agent.mV[0], second_pos_agent.mV[1], second_pos_agent.mV[2]);
			glScalef(0.1f, 0.1f, 0.1f);
			gSphere.render();
			glPopMatrix();
		}
		{
			LLGLSNoTexture no_texture;
			color = mColor;
			color.mV[3] *= 0.25f*alpha_factor;
			glColor4fv(color.mV);
			glBegin(GL_LINES);
			glVertex3fv(first_line_pos.mV);
			glVertex3fv(second_line_pos.mV);
			glEnd();
		}
	}
	
	LLFontGL *fontp = LLFontGL::sSansSerif;
	if (mLabel.size())
	{
		hud_render_utf8text(mLabel, center_agent, *fontp, LLFontGL::NORMAL, -0.5f*fontp->getWidthF32(mLabel), 0, LLColor4::white, FALSE);
	}
}

void LLHUDConnector::setZCompare(const BOOL zcompare)
{
	mZCompare = zcompare;
}

void LLHUDConnector::setColors(const LLColor4 &color, const LLColor4 &first_color, const LLColor4 &second_color)
{
	mColor = color;
	mFirstColor = first_color;
	mSecondColor = second_color;
}

void LLHUDConnector::setDoFade(const BOOL do_fade)
{
	mDoFade = do_fade;
}

void LLHUDConnector::setEndpoints(const BOOL &first, const BOOL &second)
{
	mDrawFirst = first;
	mDrawSecond = second;
}
