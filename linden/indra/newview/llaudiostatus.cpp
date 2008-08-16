/** 
 * @file llaudiostatus.cpp
 * @brief Audio channel allocation information
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#include "llaudiostatus.h"

#include "linked_lists.h"
#include "llmath.h"		// clampf()
#include "llgl.h"
#include "llview.h"
#include "llfontgl.h"
#include "llcircuit.h"
#include "message.h"
#include "sound_ids.h"
#include "audioengine.h"

#include "llui.h"
#include "llagent.h"
#include "llviewercontrol.h"
#include "viewer.h"

#include "llglheaders.h"

//
// Imported globals
//
extern LLMessageSystem* gMessageSystem;
extern LLAudioEngine*   gAudiop;

//
// Constants
//


LLAudiostatus::LLAudiostatus(const std::string& name, const LLRect& rect)
		:	LLView(name, rect, FALSE)
{
	mPage = 0;
}

//
// Functions
//

EWidgetType LLAudiostatus::getWidgetType() const
{
	return WIDGET_TYPE_AUDIO_STATUS;
}

LLString LLAudiostatus::getWidgetTag() const
{
	return LL_AUDIOSTATUS_TAG;
}

const F32 AUDIOSTATUS_SIZE = 64;

void LLAudiostatus::draw()
{
	/*
	mPage = gSavedSettings.getS32("AudioInfoPage");

	if (!getVisible() || !mPage)
	{
		return;
	}
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// We're going to be drawing flat shaded rectangles
	{
		LLGLSNoTexture gls_ui_no_texture;

		// Draw background black rectangle
		glColor4f(0.f, 0.f, 0.f, 0.3f);
		gl_rect_2d(0, 0, 325, -340);   // -y is down (don't know why)
	}
		
	glPopMatrix();
	glPushMatrix();

    S32 channel;
	char out_str[64];
	LLColor4 color; 

	if (gAudiop)
	{
		for (channel = 0; channel < MAX_BUFFERS; channel++)
		{
			if (gAudiop->getFlags(channel) & LL_SOUND_FLAG_LOOP)
			{
				color.setVec(0.5f,0.5f,1.f,1.f);
			}
			else
			{
				color.setVec(1.f,1.f,1.f,1.f);
			}

			sprintf(out_str,"%d:",channel);

			U8 render_style = LLFontGL::NORMAL;

			if (gAudiop->getFlags(channel) & LL_SOUND_FLAG_SYNC_MASTER)
			{
				render_style |= LLFontGL::BOLD;
			}

			if (gAudiop->getFlags(channel) & LL_SOUND_FLAG_SYNC_SLAVE)
			{
				render_style |= LLFontGL::ITALIC;
			}

			if (gAudiop->getFlags(channel) & LL_SOUND_FLAG_SYNC_PENDING)
			{
				render_style |= LLFontGL::UNDERLINE;
			}

			LLFontGL::sMonospace->render(out_str, 0, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP, render_style, S32_MAX, S32_MAX, NULL);

//			render_style = LLFontGL::NORMAL;
//			if (gAudiop->mCurrentSyncMaster == channel)
//			{
//				render_style |= LLFontGL::BOLD;
//			}

			if (!gAudiop->mBufferInUse[channel])
			{
				color.setVec(0.5f,0.5f,0.5f,1.f);
			}
			else if (gAudiop->isPlaying(channel))
			{
				color.setVec(0.f,1.f,0.f,1.f);
			}
			else
			{
				color.setVec(1.f,0.f,0.f,1.f);
			}

			if (gAudiop->mWindBufferID ==channel)
			{
				strcpy(out_str,"wind DMO");
			}
			else if (gAudiop->mWaterBufferID ==channel)
			{
				strcpy(out_str,"water DMO");
			}
			else if (gAudiop->mGUID[channel] == SND_CHIRP)
			{
				strcpy(out_str,"chirp");				
			}
			else if (gAudiop->mGUID[channel] == SND_SHOT)
			{
				strcpy(out_str,"shot");				
			}
			else if (gAudiop->mGUID[channel] == SND_MORTAR)
			{
				strcpy(out_str,"mortar");				
			}
			else if (gAudiop->mGUID[channel] == SND_HIT)
			{
				strcpy(out_str,"hit");				
			}
			else if (gAudiop->mGUID[channel] == SND_EXPLOSION)
			{
				strcpy(out_str,"explosion");				
			}
			else if (gAudiop->mGUID[channel] == SND_BOING)
			{
				strcpy(out_str,"boing");				
			}
			else if (gAudiop->mGUID[channel] == SND_MUNCH)
			{
				strcpy(out_str,"munch");				
			}
			else if (gAudiop->mGUID[channel] == SND_PUNCH)
			{
				strcpy(out_str,"punch");				
			}
			else if (gAudiop->mGUID[channel] == SND_SPLASH)
			{
				strcpy(out_str,"splash");				
			}
			else if (gAudiop->mGUID[channel] == SND_CLICK)
			{
				strcpy(out_str,"click");				
			}
			else if (gAudiop->mGUID[channel] == SND_ARROW_SHOT)
			{
				strcpy(out_str,"arrow");				
			}
			else if (gAudiop->mGUID[channel] == SND_ARROW_THUD)
			{
				strcpy(out_str,"arrow thud");				
			}
			else if (gAudiop->mGUID[channel] == SND_SILENCE)
			{
				strcpy(out_str,"silence");				
			}
			else if (gAudiop->mGUID[channel] == SND_WELCOME)
			{
				strcpy(out_str,"welcome");				
			}
			else if (gAudiop->mGUID[channel] == SND_WELCOME)
			{
				strcpy(out_str,"welcome");				
			}
			else if (gAudiop->mGUID[channel] == SND_READY_FOR_BATTLE)
			{
				strcpy(out_str,"ready for battle");				
			}
			else if (gAudiop->mGUID[channel] == SND_SQUISH)
			{
				strcpy(out_str,"squish");				
			}
			else if (gAudiop->mGUID[channel] == SND_FOOTSTEPS)
			{
				strcpy(out_str,"footsteps");				
			}
			else if (gAudiop->mGUID[channel] == SND_BALL_COLLISION)
			{
				strcpy(out_str,"ball collision");				
			}
			else
			{
				gAudiop->mGUID[channel].toString(out_str);
			}

			out_str[8] = 0;
			LLFontGL::sMonospace->render(out_str, 23, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP, render_style, S32_MAX, S32_MAX, NULL);

			if (mPage == 1)
			{
				LLVector3 pos = gAudiop->getSourcePos(channel);
				sprintf(out_str,"%6.2f %6.2f %6.2f",pos.mV[VX],pos.mV[VY],pos.mV[VZ]);
				LLFontGL::sMonospace->render(out_str, 103, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP);
				
				F32 gain = gAudiop->getSourceGain_dB(channel);
				sprintf(out_str,"%7.2f",gain);
				LLFontGL::sMonospace->render(out_str, 260, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP);
				
				sprintf(out_str,"%X", gAudiop->mPriority[channel] >> 28);
				LLFontGL::sMonospace->render(out_str, 315, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP);
			}
			else if (mPage == 4)
			{
				F32 indicator_width = 240;
				
				glPushMatrix();
				// We're going to be drawing flat shaded rectangles
				LLGLSNoTexture gls_no_texture;				
				// Draw background black rectangle
				glColor4f(0.f, 0.f, 0.f, 1.0f);
				
				F32 length = gAudiop->getSourceLength(channel);
				length = length/(44100.0*2.0);  // length in seconds
				length = length/10.f;           // rescale per maximum length

				gl_rect_2d(80, (-channel*10)-1, 80 + llfloor((F32)(indicator_width * length)), (-channel*10)-9);   // -y is down (don't know why)
				
				F32 current_pointer = gAudiop->getSourceCurrentSample(channel);
				current_pointer = current_pointer/(44100.0*2.0);  // length in seconds
				current_pointer = current_pointer/10.f;           // rescale per maximum length

				if (!gAudiop->mBufferInUse[channel])
				{
					glColor4f(0.5f,0.5f,0.5f,1.f);
				}
				else if (gAudiop->isPlaying(channel))
				{
					glColor4f(0.f,1.f,0.f,1.f);
				}
				else
				{
					glColor4f(1.f,0.f,0.f,1.f);
				}

				gl_rect_2d(80, (-channel*10)-1, 80 + llfloor((F32)(indicator_width * current_pointer)), (-channel*10)-9);   // -y is down (don't know why)
				
				glPopMatrix();
				{
				LLGLSUIDefault gls_ui;

				color.setVec(1.f,1.f,1.f,1.f);

				sprintf(out_str,"%6.3f",length*10.f);
				LLFontGL::sMonospace->render(out_str, 200, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP, render_style, S32_MAX, S32_MAX, NULL);

				if (!(gAudiop->mVO_ID[channel].isNull()))
				{
					gAudiop->mVO_ID[channel].toString(out_str);
					out_str[8] = 0;
					LLFontGL::sMonospace->render(out_str, 260, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP, render_style, S32_MAX, S32_MAX, NULL);
				}
				if (gAudiop->mQueuedTrigger[channel])
				{
					gAudiop->mQueuedTrigger[channel]->mID.toString(out_str);
					out_str[8] = 0;
					LLFontGL::sMonospace->render(out_str, 320, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP, render_style, S32_MAX, S32_MAX, NULL);
				}
				}
			}
			else 
			{
				S32 volume;
				S32 freq;
				S32 inside;
				S32 outside;
				LLVector3 orient;
				S32 out_volume;
				F32 min_dist;
				F32 max_dist;
				
				gAudiop->get3DParams(channel, &volume, &freq, &inside, &outside, &orient, &out_volume, &min_dist, &max_dist);

				if (mPage == 2)
				{
					sprintf(out_str,"%4.2f %4.2f %4.2f",orient.mV[VX],orient.mV[VY],orient.mV[VZ]);
					LLFontGL::sMonospace->render(out_str, 103, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP);
					
					sprintf(out_str,"%4d %4d",inside, outside);
					LLFontGL::sMonospace->render(out_str, 210, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP);
					
					sprintf(out_str,"%6d", out_volume);
					LLFontGL::sMonospace->render(out_str, 280, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP);
				}
				else // mPage == 3
				{
					F32 distance = 0.f;
					if (gAudiop->isSourceAmbient(channel))
					{
						distance = dist_vec(gAudiop->getSourcePos(channel),LLVector3::zero);
					}
					else
					{
						distance = dist_vec(gAudiop->getSourcePos(channel),gAudiop->getListenerPos());
					}

					sprintf(out_str,"%6.2f   %6.2f %6.2f",distance, min_dist, max_dist);
					LLFontGL::sMonospace->render(out_str, 103, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP);
					
					F32 gain = gAudiop->getSourceGain_dB(channel);
					sprintf(out_str,"%7.2f",gain);
					LLFontGL::sMonospace->render(out_str, 280, -channel*10, color, LLFontGL::LEFT, LLFontGL::TOP);
  				}
			}
		}
		LLFontGL::sMonospace->render("Listener", 23, -(channel*10+5), color, LLFontGL::LEFT, LLFontGL::TOP);


		if (mPage == 1)
		{
			LLVector3 lpos = gAudiop->getListenerPos();
			sprintf(out_str,"%6.2f %6.2f %6.2f",lpos.mV[VX],lpos.mV[VY],lpos.mV[VZ]);
			LLFontGL::sMonospace->render(out_str, 103, -(channel*10+5), color, LLFontGL::LEFT, LLFontGL::TOP);

			F32 gain = gAudiop->getMasterGain_dB();
			sprintf(out_str,"%7.2f",gain);
			LLFontGL::sMonospace->render(out_str, 280, -(channel*10+5), color, LLFontGL::LEFT, LLFontGL::TOP);
		}
		else if (mPage == 2)
		{
			sprintf(out_str,"cone orient    inner outer gain");
			LLFontGL::sMonospace->render(out_str, 103, -(channel*10+5), color, LLFontGL::LEFT, LLFontGL::TOP);
		}
		else if (mPage == 3)
		{
			sprintf(out_str,"distance    min    max      gain");
			LLFontGL::sMonospace->render(out_str, 103, -(channel*10+5), color, LLFontGL::LEFT, LLFontGL::TOP);			
		}
	}
	else
	{
		LLFontGL::sMonospace->render("No Audio Engine available", 50 , -175, 
			LLColor4(1.f,0.1f,0.1f,1.f),
			LLFontGL::LEFT, LLFontGL::TOP);		
	}
	
	glPopMatrix();
	*/
}






