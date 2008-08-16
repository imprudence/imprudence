/** 
 * @file lllogchat.cpp
 * @brief LLLogChat class implementation
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

#include "lllogchat.h"

const S32 LOG_RECALL_SIZE = 2048;

//static
LLString LLLogChat::makeLogFileName(LLString filename)
{
	
	filename = gDirUtilp->getExpandedFilename(LL_PATH_PER_ACCOUNT_CHAT_LOGS,filename.c_str());
	filename += ".txt";
	return filename;
}

//static
void LLLogChat::saveHistory(LLString filename, LLString line)
{
	if(!filename.size())
	{
		llinfos << "Filename is Empty!" << llendl;
		return;
	}

	FILE* fp = LLFile::fopen(LLLogChat::makeLogFileName(filename).c_str(), "a"); 		/*Flawfinder: ignore*/
	if (!fp)
	{
		llinfos << "Couldn't open chat history log!" << llendl;
	}
	else
	{
		fprintf(fp, "%s\n", line.c_str());
		
		fclose (fp);
	}
}

void LLLogChat::loadHistory(LLString filename , void (*callback)(LLString,void*), void* userdata)
{
	if(!filename.size())
	{
		llerrs << "Filename is Empty!" << llendl;
	}

	FILE* fptr = LLFile::fopen(makeLogFileName(filename).c_str(), "r");		/*Flawfinder: ignore*/
	if (!fptr)
	{
		return;			//No previous conversation with this name.
	}
	else
	{
		char buffer[LOG_RECALL_SIZE];		/*Flawfinder: ignore*/
		char *bptr;
		S32 len;
		bool firstline=TRUE;

		if ( fseek(fptr, (LOG_RECALL_SIZE - 1) * -1  , SEEK_END) )		
		{	//File is smaller than recall size.  Get it all.
			firstline = FALSE;
			if ( fseek(fptr, 0, SEEK_SET) )
			{
				fclose(fptr);
				return;
			}
		}

		while ( fgets(buffer, LOG_RECALL_SIZE, fptr)  && !feof(fptr) ) 
		{
			len = strlen(buffer) - 1;		/*Flawfinder: ignore*/
			for ( bptr = (buffer + len); (*bptr == '\n' || *bptr == '\r') && bptr>buffer; bptr--)	*bptr='\0';
			
			if (!firstline)
			{
				callback(buffer,userdata);
			}
			else
			{
				firstline = FALSE;
			}
		}
		callback("-- End of Log ---",userdata);
		
		fclose(fptr);
	}
}
