/** 
 * @file lldir_mac.cpp
 * @brief Implementation of directory utilities for Mac OS X
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

#if LL_DARWIN

#include "linden_common.h"

#include "lldir_mac.h"
#include "llerror.h"
#include "llrand.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glob.h>

#include <Carbon/Carbon.h>

// --------------------------------------------------------------------------------

static OSStatus CFCreateDirectory(FSRef	*parentRef, CFStringRef name, FSRef *newRef)
{
	OSStatus		result = noErr;
	HFSUniStr255	uniStr;
	
	uniStr.length = CFStringGetLength(name);
	CFStringGetCharacters(name, CFRangeMake(0, uniStr.length), uniStr.unicode);
	result = FSMakeFSRefUnicode(parentRef, uniStr.length, uniStr.unicode, kTextEncodingMacRoman, newRef);
	if (result != noErr)
	{
		result = FSCreateDirectoryUnicode(parentRef, uniStr.length, uniStr.unicode, 0, NULL, newRef, NULL, NULL);
	}

	return result;
}

// --------------------------------------------------------------------------------

static void CFStringRefToLLString(CFStringRef stringRef, std::string &llString, bool releaseWhenDone)
{
	if (stringRef)
	{
		long	bufferSize = CFStringGetLength(stringRef) + 1;
		char* buffer = new char[bufferSize];
		memset(buffer, 0, bufferSize);
		if (CFStringGetCString(stringRef, buffer, bufferSize, kCFStringEncodingUTF8))
			llString = buffer;
		delete[] buffer;
		if (releaseWhenDone)
			CFRelease(stringRef);
	}
}

// --------------------------------------------------------------------------------

static void CFURLRefToLLString(CFURLRef urlRef, std::string &llString, bool releaseWhenDone)
{
	if (urlRef)
	{
		CFURLRef	absoluteURLRef = CFURLCopyAbsoluteURL(urlRef);
		if (absoluteURLRef)
		{
			CFStringRef	stringRef = CFURLCopyFileSystemPath(absoluteURLRef, kCFURLPOSIXPathStyle);
			CFStringRefToLLString(stringRef, llString, true);
			CFRelease(absoluteURLRef);
		}
		if (releaseWhenDone)
			CFRelease(urlRef);
	}
}

// --------------------------------------------------------------------------------

static void FSRefToLLString(FSRef *fsRef, std::string &llString)
{
	OSStatus	error = noErr;
	char		path[MAX_PATH];
	
	error = FSRefMakePath(fsRef, (UInt8*) path, sizeof(path));
	if (error == noErr)
		llString = path;
}

// --------------------------------------------------------------------------------

LLDir_Mac::LLDir_Mac()
{
	mDirDelimiter = "/";
	mCurrentDirIndex = -1;
	mCurrentDirCount = -1;
	
	CFBundleRef		mainBundleRef = NULL;
	CFURLRef		executableURLRef = NULL;
	CFStringRef		stringRef = NULL;
	OSStatus		error = noErr;
	FSRef			fileRef;
	CFStringRef		secondLifeString = CFSTR("Imprudence");
	
	mainBundleRef = CFBundleGetMainBundle();
		
	executableURLRef = CFBundleCopyExecutableURL(mainBundleRef);
	
	if (executableURLRef != NULL)
	{
		// mExecutablePathAndName
		CFURLRefToLLString(executableURLRef, mExecutablePathAndName, false);
		
		// mExecutableFilename
		stringRef = CFURLCopyLastPathComponent(executableURLRef);
		CFStringRefToLLString(stringRef, mExecutableFilename, true);
		
		// mExecutableDir
		CFURLRef	executableParentURLRef = CFURLCreateCopyDeletingLastPathComponent(NULL, executableURLRef);
		CFURLRefToLLString(executableParentURLRef, mExecutableDir, true);
		
		// mAppRODataDir
		CFURLRef	resourcesURLRef = CFBundleCopyResourcesDirectoryURL(mainBundleRef);
		CFURLRefToLLString(resourcesURLRef, mAppRODataDir, true);
		
		// mOSUserDir
		error = FSFindFolder(kUserDomain, kApplicationSupportFolderType, true, &fileRef);
		if (error == noErr)
		{
			FSRef	newFileRef;
			
			// Create the directory
			error = CFCreateDirectory(&fileRef, secondLifeString, &newFileRef);
			if (error == noErr)
			{
				// Save the full path to the folder
				FSRefToLLString(&newFileRef, mOSUserDir);
				
				// Create our sub-dirs
				(void) CFCreateDirectory(&newFileRef, CFSTR("data"), NULL);
				//(void) CFCreateDirectory(&newFileRef, CFSTR("cache"), NULL);
				(void) CFCreateDirectory(&newFileRef, CFSTR("logs"), NULL);
				(void) CFCreateDirectory(&newFileRef, CFSTR("user_settings"), NULL);
				(void) CFCreateDirectory(&newFileRef, CFSTR("browser_profile"), NULL);
			}
		}
		
		//mOSCacheDir
		FSRef cacheDirRef;
		error = FSFindFolder(kUserDomain, kCachedDataFolderType, true, &cacheDirRef);
		if (error == noErr)
		{
			FSRefToLLString(&cacheDirRef, mOSCacheDir);
			(void)CFCreateDirectory(&cacheDirRef, CFSTR("ImprudenceExperimental"),NULL);
		}
		
		// mOSUserAppDir
		mOSUserAppDir = mOSUserDir;
		
		// mTempDir
		error = FSFindFolder(kOnAppropriateDisk, kTemporaryFolderType, true, &fileRef);
		if (error == noErr)
		{
			FSRef	tempRef;
			error = CFCreateDirectory(&fileRef, secondLifeString, &tempRef);
			if (error == noErr)
				FSRefToLLString(&tempRef, mTempDir);
		}
		
		mWorkingDir = getCurPath();

		mLLPluginDir = mAppRODataDir + mDirDelimiter + "llplugin";
				
		CFRelease(executableURLRef);
		executableURLRef = NULL;
	}
}

LLDir_Mac::~LLDir_Mac()
{
}

// Implementation


void LLDir_Mac::initAppDirs(const std::string &app_name)
{
	mCAFile = getExpandedFilename(LL_PATH_APP_SETTINGS, "CA.pem");

	//dumpCurrentDirectories();
}

U32 LLDir_Mac::countFilesInDir(const std::string &dirname, const std::string &mask)
{
	U32 file_count = 0;
	glob_t g;

	std::string tmp_str;
	tmp_str = dirname;
	tmp_str += mask;
	
	if(glob(tmp_str.c_str(), GLOB_NOSORT, NULL, &g) == 0)
	{
		file_count = g.gl_pathc;

		globfree(&g);
	}

	return (file_count);
}

// get the next file in the directory
// automatically wrap if we've hit the end
BOOL LLDir_Mac::getNextFileInDir(const std::string &dirname, const std::string &mask, std::string &fname, BOOL wrap)
{
	glob_t g;
	BOOL result = FALSE;
	fname = "";
	
	if(!(dirname == mCurrentDir))
	{
		// different dir specified, close old search
		mCurrentDirIndex = -1;
		mCurrentDirCount = -1;
		mCurrentDir = dirname;
	}
	
	std::string tmp_str;
	tmp_str = dirname;
	tmp_str += mask;

	if(glob(tmp_str.c_str(), GLOB_NOSORT, NULL, &g) == 0)
	{
		if(g.gl_pathc > 0)
		{
			if(g.gl_pathc != mCurrentDirCount)
			{
				// Number of matches has changed since the last search, meaning a file has been added or deleted.
				// Reset the index.
				mCurrentDirIndex = -1;
				mCurrentDirCount = g.gl_pathc;
			}
	
			mCurrentDirIndex++;
	
			if((mCurrentDirIndex >= g.gl_pathc) && wrap)
			{
				mCurrentDirIndex = 0;
			}
			
			if(mCurrentDirIndex < g.gl_pathc)
			{
//				llinfos << "getNextFileInDir: returning number " << mCurrentDirIndex << ", path is " << g.gl_pathv[mCurrentDirIndex] << llendl;

				// The API wants just the filename, not the full path.
				//fname = g.gl_pathv[mCurrentDirIndex];

				char *s = strrchr(g.gl_pathv[mCurrentDirIndex], '/');
				
				if(s == NULL)
					s = g.gl_pathv[mCurrentDirIndex];
				else if(s[0] == '/')
					s++;
					
				fname = s;
				
				result = TRUE;
			}
		}
		
		globfree(&g);
	}
	
	return(result);
}

// get a random file in the directory
void LLDir_Mac::getRandomFileInDir(const std::string &dirname, const std::string &mask, std::string &fname)
{
	S32 which_file;
	glob_t g;
	fname = "";
	
	std::string tmp_str;
	tmp_str = dirname;
	tmp_str += mask;
	
	if(glob(tmp_str.c_str(), GLOB_NOSORT, NULL, &g) == 0)
	{
		if(g.gl_pathc > 0)
		{
			
			which_file = ll_rand(g.gl_pathc);
	
//			llinfos << "getRandomFileInDir: returning number " << which_file << ", path is " << g.gl_pathv[which_file] << llendl;
			// The API wants just the filename, not the full path.
			//fname = g.gl_pathv[which_file];

			char *s = strrchr(g.gl_pathv[which_file], '/');
			
			if(s == NULL)
				s = g.gl_pathv[which_file];
			else if(s[0] == '/')
				s++;
				
			fname = s;
		}
		
		globfree(&g);
	}
}

S32 LLDir_Mac::deleteFilesInDir(const std::string &dirname, const std::string &mask)
{
	glob_t g;
	S32 result = 0;
	
	std::string tmp_str;
	tmp_str = dirname;
	tmp_str += mask;
	
	if(glob(tmp_str.c_str(), GLOB_NOSORT, NULL, &g) == 0)
	{
		int i;
		
		for(i = 0; i < g.gl_pathc; i++)
		{
//			llinfos << "deleteFilesInDir: deleting number " << i << ", path is " << g.gl_pathv[i] << llendl;

			if(unlink(g.gl_pathv[i]) != 0)
			{
				result = errno;

				llwarns << "Problem removing " << g.gl_pathv[i] << " - errorcode: "
					<< result << llendl;
			}
		}

		globfree(&g);
	}
	
	return(result);
}

std::string LLDir_Mac::getCurPath()
{
	char tmp_str[LL_MAX_PATH];	/* Flawfinder: ignore */ 
	getcwd(tmp_str, LL_MAX_PATH);
	return tmp_str;
}



BOOL LLDir_Mac::fileExists(const std::string &filename) const
{
	struct stat stat_data;
	// Check the age of the file
	// Now, we see if the files we've gathered are recent...
	int res = stat(filename.c_str(), &stat_data);
	if (!res)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*virtual*/ std::string LLDir_Mac::getLLPluginLauncher()
{
	return gDirUtilp->getAppRODataDir() + gDirUtilp->getDirDelimiter() +
		"SLPlugin.app/Contents/MacOS/SLPlugin";
}

/*virtual*/ std::string LLDir_Mac::getLLPluginFilename(std::string base_name)
{
	return gDirUtilp->getLLPluginDir() + gDirUtilp->getDirDelimiter() +
		base_name + ".dylib";
}


#endif // LL_DARWIN
