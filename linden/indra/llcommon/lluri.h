/** 
 * @file lluri.h
 * @author Phoenix
 * @date 2006-02-05
 * @brief Declaration of the URI class.
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

#ifndef LL_LLURI_H
#define LL_LLURI_H

#include <string>

class LLSD;

/** 
 *
 * LLURI instances are immutable
 * See: http://www.ietf.org/rfc/rfc3986.txt
 *
 */
class LLURI
{
public:
	LLURI();
	LLURI(const std::string& escaped_str);
		// construct from escaped string, as would be transmitted on the net

	~LLURI();

	static LLURI buildHTTP(const std::string& host_port,
						   const LLSD& path);
	static LLURI buildHTTP(const std::string& host_port,
						   const LLSD& path,
						   const LLSD& query);
	
	std::string asString() const;
		// the whole URI, escaped as needed

	// Parts of a URI
	// These functions return parts of the decoded URI.  The returned
	// strings are un-escaped as needed
	
	// for all schemes
	std::string scheme() const;		// ex.: "http", note lack of colon
	std::string opaque() const;		// everything after the colon

	// for schemes that follow path like syntax (http, https, ftp)
	std::string authority() const;	// ex.: "bob@host.com:80"
	std::string hostName() const;	// ex.: "host.com"
	U16 hostPort() const;			// ex.: 80, will include implicit port
	std::string path() const;		// ex.: "/abc/def", includes leading slash
//    LLSD pathArray() const;			// above decoded into an array of strings
	std::string query() const;		// ex.: "x=34", section after "?"
    LLSD queryMap() const;			// above decoded into a map
    static LLSD queryMap(std::string escaped_query_string);

	// Escaping Utilities
	static std::string escape(const std::string& str);
	static std::string unescape(const std::string& str);

private:
	std::string mScheme;
	std::string mEscapedOpaque;
	std::string mEscapedAuthority;
	std::string mEscapedPath;
	std::string mEscapedQuery;
};

#endif // LL_LLURI_H
