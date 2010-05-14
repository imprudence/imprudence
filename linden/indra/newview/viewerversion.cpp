/**
* @file viewerversion.cpp
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

#include "llviewerprecompiledheaders.h"

#include "lldir.h"
#include "llxmltree.h"
#include "viewerversion.h"


S32 ViewerVersion::sVersionMajor = 0;
S32 ViewerVersion::sVersionMinor = 0;
S32 ViewerVersion::sVersionPatch = 0;
std::string ViewerVersion::sVersionTest = "";

const std::string ViewerVersion::sViewerName = "Imprudence";

ViewerVersion::ViewerVersion()
{
}

bool ViewerVersion::initViewerVersion()
{
	std::string file_path = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "viewerversion.xml");

	if (!gDirUtilp->fileExists(file_path))
	{
		llwarns << "Unable to find viewerversion.xml in app_settings folder" << llendl;
		return false;
	}
	else
	{
		LLXMLNodePtr root;

		if (!LLXMLNode::parseFile(file_path, root, NULL))
		{
			llwarns << "Unable to parse version file: " << file_path << llendl;
			return false;
		}
		
		if (root.isNull()) // shouldn't ever happen
		{
			llwarns << "Error while trying to read viewerversion.xml" << llendl;
			return false;
		}

		LLXMLNodePtr child_nodep = root->getFirstChild();
		while (child_nodep.notNull())
		{
			child_nodep->getAttributeS32("version_major", sVersionMajor);
			child_nodep->getAttributeS32("version_minor", sVersionMinor);
			child_nodep->getAttributeS32("version_patch", sVersionPatch);
			child_nodep->getAttributeString("version_test", sVersionTest);

			child_nodep = child_nodep->getNextSibling();
		}

		llinfos << "Version set to: " << sVersionMajor << "." << sVersionMinor << "." << sVersionPatch << " " << sVersionTest << llendl;

		return true;
	}
}
