/**
 * @file viewerinfo.cpp
 * @brief Functions for querying the viewer name, version, and other info.
 * @author Jacek Antonelli
 *
 * Copyright (c) 2010-2011, Jacek Antonelli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "llviewerprecompiledheaders.h"

#include "viewerinfo.h"

namespace ViewerInfo
{

	// These are intentionally defined here instead of in the header,
	// because they should NOT be read directly. Use the functions.
	const std::string NAME  = "Imprudence";
	const std::string VARNT = "";
	const S32         MAJOR = 1;
	const S32         MINOR = 4;
	const S32         PATCH = 1;
	const S32         RLEAS = 1; // increment for each beta/RC/release
	const std::string EXTRA = "beta 1";

	// Mac OS X bundle identifier. Should match the one in Info.plist.
	const std::string BUNDLE_ID = "org.imprudenceviewer.viewer";


	const std::string& viewerName()
	{
		return NAME;
	}

	const std::string& viewerVariant()
	{
		return VARNT;
	}

	const std::string& nameWithVariant()
	{
		static std::string s;
		if (!s.empty())
		{
			return s;
		}

		if (VARNT.empty())
		{
			s = NAME;
		}
		else
		{
			s = NAME + " " + VARNT;
		}

		return s;
	}

	S32 versionMajor()
	{
		return MAJOR;
	}

	S32 versionMinor()
	{
		return MINOR;
	}

	S32 versionPatch()
	{
		return PATCH;
	}

	S32 versionRelease()
	{
		return RLEAS;
	}

	const std::string& versionExtra()
	{
		return EXTRA;
	}

	const std::string& versionNumbers3()
	{
		static std::string s = llformat("%d.%d.%d", MAJOR, MINOR, PATCH);
		return s;
	}

	const std::string& versionNumbers4()
	{
		static std::string s = llformat("%d.%d.%d.%d",
		                                MAJOR, MINOR, PATCH, RLEAS);
		return s;
	}

	const std::string& prettyVersion()
	{
		static std::string s;
		if (s.length() > 0)
		{
			return s;
		}

		s = versionNumbers3();

		if (EXTRA.length() > 0)
		{
			s += " " + EXTRA;
		}

		return s;
	}

	const std::string& prettyInfo()
	{
		static std::string s = nameWithVariant() + " " + prettyVersion();
		return s;
	}

	const std::string& terseInfo()
	{
		static std::string s = nameWithVariant() + " " + versionNumbers4();
		return s;
	}

	const std::string& bundleID()
	{
		return BUNDLE_ID;
	}

}
