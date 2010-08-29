/**
* @file viewerversion.h
* @brief set the viewer version in xml
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2010, McCabe Maxsted
*
* Imprudence Viewer Source Code
* The source code in this file ("Source Code") is provided to you
* under the terms of the GNU General Public License, version 2.0
* ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
* this distribution, or online at
* http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
* ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
* WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
* COMPLETENESS OR PERFORMANCE.
* $/LicenseInfo$
*/

#ifndef VIEWERVERSION_H
#define VIEWERVERSION_H

#include "llversionviewer.h" // for LL versioning only

class ViewerVersion
{
public:
	ViewerVersion();
	/*virtual*/ ~ViewerVersion();

	static bool initViewerVersion();

	// Returns the major version of Imprudence
	static S32			getImpMajorVersion() { return sVersionMajor; }
	// Returns the minor version of Imprudence
	static S32			getImpMinorVersion() { return sVersionMinor; }
	// Returns the patch version of Imprudence
	static S32			getImpPatchVersion() { return sVersionPatch; }
	// Returns the test version of Imprudence
	static std::string  getImpTestVersion() { return sVersionTest; }
	// Returns the name of the viewer. Currently always "Imprudence"
	static std::string	getImpViewerName() { return sViewerName; }

	// Returns the major version of Second Life
	static S32			getLLMajorVersion() { return LL_VERSION_MAJOR; }
	// Returns the minor version of Second Life
	static S32			getLLMinorVersion() { return LL_VERSION_MINOR; }
	// Returns the patch version of Second Life
	static S32			getLLPatchVersion() { return LL_VERSION_PATCH; }
	// Returns the build version of Second Life
	static S32		    getLLBuildVersion() { return LL_VERSION_BUILD; }
	// Returns the name of the LL viewer ("Second Life")
	static std::string	getLLViewerName() { return LL_VIEWER_NAME; }

	// Note that the viewer channel is set in settings.xml
	// as VersionChannelName. LL_VIEWER_CHANNEL is not 
	// used in Imprudence

private:
	static S32			sVersionMajor;
	static S32			sVersionMinor;
	static S32			sVersionPatch;
	static std::string	sVersionTest;
	
	static const std::string  sViewerName;
};

#endif // VIEWERVERSION_H
