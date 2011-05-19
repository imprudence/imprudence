/**
 * @file viewerinfo.h
 * @brief Functions for querying the viewer name, version, and other info.
 * @author Jacek Antonelli
 *
 * Copyright (c) 2010-2011, Jacek Antonelli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
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

#ifndef VERSIONINFO_H
#define VERSIONINFO_H

#include "linden_common.h"

namespace ViewerInfo
{
	/// Returns the name of the viewer.
	const std::string& viewerName();

	/// Returns the viewer variant (e.g. "Experimental").
	/// May be empty, if no variant string was set.
	const std::string& viewerVariant();

	/// Returns a string with the viewer name and variant
	/// (if it has one).
	const std::string& nameWithVariant();

	/// Returns the major (first) version number.
	/// This number increases for each major release, and the
	/// minor, patch, and release numbers are reset to zero.
	S32 versionMajor();

	/// Returns the minor (second) version number.
	/// This number increases for each minor release, and the
	/// patch and release numbers are reset to zero.
	S32 versionMinor();

	/// Returns the patch (third) version number.
	/// This number increases for each patch (bugfix) release,
	/// and the release number is reset to zero.
	S32 versionPatch();

	/// Returns the release (fourth) version number.
	/// This number increases for each beta, release candidate,
	/// and final release.
	S32 versionRelease();

	/// Returns the extra version string (e.g. "beta 1", "RC1").
	/// May be empty, if no extra string was set.
	const std::string& versionExtra();

	/// Returns a three-segment dot-separated version string
  /// ("major.minor.patch"). Intended for human reading.
	const std::string& versionNumbers3();

	/// Returns a four-segment dot-separated version string
  /// ("major.minor.patch.release"). Intended for computer use, e.g.
  /// login channel or version number comparison.
	const std::string& versionNumbers4();

	/// Returns the three-segment version number with extra version
	/// string (if not empty). Intended for human reading.
	const std::string& prettyVersion();

	/// Returns the viewer name, variant (if not empty), and pretty
	/// version. Intended for human reading.
	const std::string& prettyInfo();

	/// Returns the the viewer name, variant (if not empty), and
	/// 4-segment version. Intended for computer use, e.g. login channel
	/// or version number comparison.
	const std::string& terseInfo();

	/// Returns a string with the viewer's Mac OS X bundle identifier.
	const std::string& bundleID();

}

#endif // VERSIONINFO_H
