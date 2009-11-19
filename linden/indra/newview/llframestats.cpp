/** 
 * @file llframestats.cpp
 * @brief LLFrameStats class implementation
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
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
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#include "llframestats.h"
#include "v4color.h"
#include "pipeline.h"
#include "llviewerobjectlist.h"
#include "lldrawpool.h"
#include "lldir.h"

LLFrameStats gFrameStats;

// static
std::string LLFrameStats::sStatLabels[NUM_STATS] = {
					"IdleNetwork",
					"AgentMisc",
					"ObjectUpdate",
					"CleanDead",
					"UpdateEffects",
					"ImageUpdate",
					"UpdateMove",
					"UpdateParticles",
					"UpdateCull",
					"UpdateGeom",
					"Audio",
					"UpdateTexStats",
					"StateSort",
					"Rebuild",
					"RenderSync",
					"RenderGeom",
					"RenderUI",
					"MiscEnd"
					};

LLColor4 LLFrameStats::sStatColors[NUM_STATS] = {
										LLColor4(0.0f, 0.5f, 1.0f, 0.5f), // IdleNetwork
										LLColor4(0.0f, 0.5f, 0.0f, 0.5f), // AgentMisc
										LLColor4(0.0f, 0.0f, 1.0f, 0.5f), // ObjectUpdate
										LLColor4(0.5f, 0.25f,0.5f, 0.5f), // CleanDead
										LLColor4(0.5f, 0.5f, 0.5f, 0.5f), // UpdateEffects
										LLColor4(0.5f, 1.0f, 0.0f, 0.5f), // ImageUpdate
										LLColor4(1.0f, 1.0f, 0.0f, 0.5f), // UpdateMove
										LLColor4(0.0f, 0.0f, 1.0f, 0.5f), // UpdateParticles
										LLColor4(1.0f, 0.0f, 0.0f, 0.5f), // UpdateCull
										LLColor4(0.5f, 0.0f, 0.0f, 0.5f), // UpdateGeom
										LLColor4(0.0f, 0.5f, 0.5f, 0.5f), // Audio
										LLColor4(0.5f, 1.0f, 1.0f, 0.5f), // UpdateTexStats
										LLColor4(0.0f, 0.0f, 0.5f, 0.5f), // StateSort
										LLColor4(1.0f, 0.0f, 1.0f, 0.5f), // Rebuild
										LLColor4(0.0f, 0.5f, 1.0f, 0.5f), // RenderSync
										LLColor4(1.0f, 1.0f, 1.0f, 0.5f), // RenderGeom
										LLColor4(0.5f, 0.5f, 0.5f, 0.5f), // RenderUI
										LLColor4(1.0f, 0.0f, 0.0f, 0.5f) // MiscEnd
										};

LLFrameStats::LLFrameStats()
{
	mCurrentStat = NUM_STATS;
	mTrackStats = FALSE;
	mStopTime = FALSE;
	mUseTimer = FALSE;
	mStopTime = 0.f;
	mFilename = "frame_stats";
}

LLFrameStats::~LLFrameStats()
{
	mFrameData.reset();
}

void LLFrameStats::start(const EStat stat)
{
	stop();
	mCurrentStat = stat;
	mStats[stat].start();
}

void LLFrameStats::stop()
{
	if (NUM_STATS != mCurrentStat)
	{
		mStats[mCurrentStat].addValue();
	}
}

extern S32 gFullObjectUpdates;
extern S32 gTerseObjectUpdates;


void LLFrameStats::addFrameData()
{
	if (NUM_STATS == mCurrentStat)
	{
		return;
	}
	if (!mTrackStats)
	{
		return;
	}

	static FrameData frame_data;

	F32 total_duration = 0.f;
	S32 i;
	for (i = 0; i < NUM_STATS; i++)
	{
		frame_data.mDuration[i] = mStats[i].getCurrentDuration();
		total_duration +=  frame_data.mDuration[i];
	}

	frame_data.mTotalDuration = total_duration;
	frame_data.mNumTriangles = gPipeline.mTrianglesDrawn;
	frame_data.mNumObjects = gObjectList.getNumObjects();
	frame_data.mNumFullUpdates = gFullObjectUpdates;
	frame_data.mNumTerseUpdates = gTerseObjectUpdates;

	gFullObjectUpdates = 0;
	gTerseObjectUpdates = 0;


	mFrameData.put(frame_data);
	if (mUseTimer)
	{
		if (mTimer.getElapsedTimeF32() > mStopTime)
		{
			llinfos << "Grabbed stats for " << mStopTime << " seconds, stopping and dumping" << llendl;
			setTrackStats(FALSE);
		}
	}
}

void LLFrameStats::dump()
{
	if (mFrameData.count())
	{
		F32 total_time = 0;
		S64 total_triangles = 0;
		S32 total_frames = mFrameData.count();
		S32 total_num_objects = 0;

		time_t cur_time;
		char time_str[24];		/* Flawfinder: ignore */
		//char *time_str;
		time(&cur_time);
		strftime(time_str, 24, "%Y.%m.%d %H:%M:%S", localtime(&cur_time));
		time_str[19] = '\n';
		time_str[20] = '\0';

		static S32 dump_count = 0;

		std::string file_with_num;
		file_with_num = llformat("fs%d.txt", dump_count);
		dump_count++;

		std::string filename;
		filename = gDirUtilp->getExpandedFilename(LL_PATH_LOGS, file_with_num);
		LLFILE *fp = LLFile::fopen(filename, "w");		/* Flawfinder: ignore */
		if (!fp)
		{
			llinfos << "Couldn't open file for dumping frame stats!" << llendl;
			return;
		}
		llinfos << "Dumping frame statistics for " << mFrameData.count() << " frames" << llendl;

		fprintf(fp, "Time\tNumTriangles\t");

		S32 i;
		for (i = 0; i < NUM_STATS; i++)
		{
			fprintf(fp, "%s\t", sStatLabels[i].c_str());
		}
		fprintf(fp, "Full Updates\tTerse Updates\tTotal Vorbis\tLong Vorbis\tNum Vorbis Decodes\t");
		fprintf(fp, "\n");

		for (i = 0; i < mFrameData.count(); i++)
		{
			total_time += mFrameData[i].mTotalDuration;
			total_triangles += mFrameData[i].mNumTriangles;
			total_num_objects += mFrameData[i].mNumObjects;

			fprintf(fp, "%f\t%d\t", mFrameData[i].mTotalDuration, mFrameData[i].mNumTriangles);
			S32 j;
			for (j = 0; j < NUM_STATS; j++)
			{
				fprintf(fp, "%f\t", mFrameData[i].mDuration[j]);
			}
			fprintf(fp, "%d\t", mFrameData[i].mNumFullUpdates);
			fprintf(fp, "%d\t", mFrameData[i].mNumTerseUpdates);
			fprintf(fp, "%f\t", mFrameData[i].mTotalVorbisTime);
			fprintf(fp, "%f\t", mFrameData[i].mLongVorbisTime);
			fprintf(fp, "%d\t", mFrameData[i].mNumVorbisDecodes);
			fprintf(fp, "\n");
		}
		fclose(fp);

		// Now dump cumulative stats
		filename = gDirUtilp->getExpandedFilename(LL_PATH_LOGS, mSummaryFilename);
		fp = LLFile::fopen(filename, "a");		/* Flawfinder: ignore */
		if (!fp)
		{
			llinfos << "Couldn't open file for dumping frame stats!" << llendl;
			return;
		}

		fprintf(fp, "Performance data summary\n");
		fputs(time_str, fp);

		fprintf(fp, "------------------------\n");
		fprintf(fp, "Total Time: %f\n", total_time);
		fprintf(fp, "Total Frames: %d\n", total_frames);
		fprintf(fp, "Total Triangles: %.0f\n", (F32)total_triangles);
		fprintf(fp, "Frames/sec: %f\n", total_frames / total_time);
		fprintf(fp, "Triangles/sec: %f\n", total_triangles/total_time);
		fprintf(fp, "Triangles/frame: %f\n", (F32)total_triangles/(F32)total_frames);
		fprintf(fp, "All Objects/frame: %f\n", (F32)total_num_objects/(F32)total_frames);
		fprintf(fp, "\n");
		fclose(fp);
	}
	mFrameData.reset();
}

void LLFrameStats::setTrackStats(const BOOL track_stats)
{
	if (mTrackStats != track_stats)
	{
		if (!track_stats)
		{
			dump();
		}
		else
		{
			llinfos << "Enabling stat logging" << llendl;
		}
	}

	if (track_stats)
	{
		// Reset the frame data
		mFrameData.reset();
	}
	mTrackStats = track_stats;
}

// static callbacks
void LLFrameStats::startLogging(void *)
{
	gFrameStats.setTrackStats(TRUE);
}

void LLFrameStats::stopLogging(void *)
{
	gFrameStats.setTrackStats(FALSE);
}

void LLFrameStats::timedLogging60(void *)
{
	gFrameStats.setTrackStats(TRUE);
	gFrameStats.mTimer.reset();
	gFrameStats.mStopTime = 60.f;
	gFrameStats.mUseTimer = TRUE;
}

void LLFrameStats::timedLogging30(void *)
{
	gFrameStats.setTrackStats(TRUE);
	gFrameStats.mTimer.reset();
	gFrameStats.mStopTime = 30.f;
	gFrameStats.mUseTimer = TRUE;
}

void LLFrameStats::timedLogging10(void *)
{
	gFrameStats.setTrackStats(TRUE);
	gFrameStats.mTimer.reset();
	gFrameStats.mStopTime = 10.f;
	gFrameStats.mUseTimer = TRUE;
}

