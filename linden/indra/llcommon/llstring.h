/** 
 * @file llstring.h
 * @brief String utility functions and std::string class.
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

#ifndef LL_LLSTRING_H
#define LL_LLSTRING_H

#include <string>
#include <cstdio>
#include <algorithm>
#include <map>

#if LL_LINUX || LL_SOLARIS
#include <wctype.h>
#include <wchar.h>
#endif

#include <string.h>

#if LL_SOLARIS
// stricmp and strnicmp do not exist on Solaris:
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

const char LL_UNKNOWN_CHAR = '?';

#if LL_DARWIN || LL_LINUX || LL_SOLARIS
// Template specialization of char_traits for U16s. Only necessary on Mac and Linux (exists on Windows already)
#include <cstring>

namespace std
{
template<>
struct char_traits<U16>
{
	typedef U16 		char_type;
	typedef int 	    int_type;
	typedef streampos 	pos_type;
	typedef streamoff 	off_type;
	typedef mbstate_t 	state_type;
	
	static void 
		assign(char_type& __c1, const char_type& __c2)
	{ __c1 = __c2; }
	
	static bool 
		eq(const char_type& __c1, const char_type& __c2)
	{ return __c1 == __c2; }
	
	static bool 
		lt(const char_type& __c1, const char_type& __c2)
	{ return __c1 < __c2; }
	
	static int 
		compare(const char_type* __s1, const char_type* __s2, size_t __n)
	{ return memcmp(__s1, __s2, __n * sizeof(char_type)); }
	
	static size_t
		length(const char_type* __s)
	{
		const char_type *cur_char = __s;
		while (*cur_char != 0)
		{
			++cur_char;
		}
		return cur_char - __s;
	}
	
	static const char_type* 
		find(const char_type* __s, size_t __n, const char_type& __a)
	{ return static_cast<const char_type*>(memchr(__s, __a, __n * sizeof(char_type))); }
	
	static char_type* 
		move(char_type* __s1, const char_type* __s2, size_t __n)
	{ return static_cast<char_type*>(memmove(__s1, __s2, __n * sizeof(char_type))); }
	
	static char_type* 
		copy(char_type* __s1, const char_type* __s2, size_t __n)
	{  return static_cast<char_type*>(memcpy(__s1, __s2, __n * sizeof(char_type))); }	/* Flawfinder: ignore */
	
	static char_type* 
		assign(char_type* __s, size_t __n, char_type __a)
	{ 
		// This isn't right.
		//return static_cast<char_type*>(memset(__s, __a, __n * sizeof(char_type))); 
		
		// I don't think there's a standard 'memset' for 16-bit values.
		// Do this the old-fashioned way.
		
		size_t __i;
		for(__i = 0; __i < __n; __i++)
		{
			__s[__i] = __a;
		}
		return __s; 
	}
	
	static char_type 
		to_char_type(const int_type& __c)
	{ return static_cast<char_type>(__c); }
	
	static int_type 
		to_int_type(const char_type& __c)
	{ return static_cast<int_type>(__c); }
	
	static bool 
		eq_int_type(const int_type& __c1, const int_type& __c2)
	{ return __c1 == __c2; }
	
	static int_type 
		eof() { return static_cast<int_type>(EOF); }
	
	static int_type 
		not_eof(const int_type& __c)
      { return (__c == eof()) ? 0 : __c; }
  };
};
#endif

class LL_COMMON_API LLStringOps
{
public:
	static char toUpper(char elem) { return toupper((unsigned char)elem); }
	static llwchar toUpper(llwchar elem) { return towupper(elem); }
	
	static char toLower(char elem) { return tolower((unsigned char)elem); }
	static llwchar toLower(llwchar elem) { return towlower(elem); }

	static bool isSpace(char elem) { return isspace((unsigned char)elem) != 0; }
	static bool isSpace(llwchar elem) { return iswspace(elem) != 0; }

	static bool isUpper(char elem) { return isupper((unsigned char)elem) != 0; }
	static bool isUpper(llwchar elem) { return iswupper(elem) != 0; }

	static bool isLower(char elem) { return islower((unsigned char)elem) != 0; }
	static bool isLower(llwchar elem) { return iswlower(elem) != 0; }

	static bool isDigit(char a) { return isdigit((unsigned char)a) != 0; }
	static bool isDigit(llwchar a) { return iswdigit(a) != 0; }

	static bool isPunct(char a) { return ispunct((unsigned char)a) != 0; }
	static bool isPunct(llwchar a) { return iswpunct(a) != 0; }

	static bool isAlnum(char a) { return isalnum((unsigned char)a) != 0; }
	static bool isAlnum(llwchar a) { return iswalnum(a) != 0; }

	static S32	collate(const char* a, const char* b) { return strcoll(a, b); }
	static S32	collate(const llwchar* a, const llwchar* b);
};

/**
 * @brief Return a string constructed from in without crashing if the
 * pointer is NULL.
 */
std::string LL_COMMON_API ll_safe_string(const char* in);
std::string LL_COMMON_API ll_safe_string(const char* in, S32 maxlen);


// Allowing assignments from non-strings into format_map_t is apparently
// *really* error-prone, so subclass std::string with just basic c'tors.
class LLFormatMapString
{
public:
	LLFormatMapString() {};
	LLFormatMapString(const char* s) : mString(ll_safe_string(s)) {};
	LLFormatMapString(const std::string& s) : mString(s) {};
	operator std::string() const { return mString; }
	bool operator<(const LLFormatMapString& rhs) const { return mString < rhs.mString; }
	std::size_t length() const { return mString.length(); }
	
private:
	std::string mString;
};

template <class T>
class LLStringUtilBase
{
public:
	typedef typename std::basic_string<T>::size_type size_type;
	
public:
	/////////////////////////////////////////////////////////////////////////////////////////
	// Static Utility functions that operate on std::strings

	static std::basic_string<T> const null;
	
	typedef std::map<LLFormatMapString, LLFormatMapString> format_map_t;
	static S32 format(std::basic_string<T>& s, const format_map_t& fmt_map);
	
	static bool isValidIndex(const std::basic_string<T>& string, size_type i)
	{
		return !string.empty() && (0 <= i) && (i <= string.size());
	}

	static void	trimHead(std::basic_string<T>& string);
	static void	trimTail(std::basic_string<T>& string);
	static void	trim(std::basic_string<T>& string)	{ trimHead(string); trimTail(string); }
	static void truncate(std::basic_string<T>& string, size_type count);

	static void	toUpper(std::basic_string<T>& string);
	static void	toLower(std::basic_string<T>& string);
	
	// True if this is the head of s.
	static BOOL	isHead( const std::basic_string<T>& string, const T* s ); 
	
	static void	addCRLF(std::basic_string<T>& string);
	static void	removeCRLF(std::basic_string<T>& string);

	static void	replaceTabsWithSpaces( std::basic_string<T>& string, size_type spaces_per_tab );
	static void	replaceNonstandardASCII( std::basic_string<T>& string, T replacement );
	static void	replaceChar( std::basic_string<T>& string, T target, T replacement );
	static void replaceString( std::basic_string<T>& string, std::basic_string<T> target, std::basic_string<T> replacement );
	
	static BOOL	containsNonprintable(const std::basic_string<T>& string);
	static void	stripNonprintable(std::basic_string<T>& string);

	/**
	 * @brief Unsafe way to make ascii characters. You should probably
	 * only call this when interacting with the host operating system.
	 * The 1 byte std::string does not work correctly.
	 * The 2 and 4 byte std::string probably work, so LLWStringUtil::_makeASCII
	 * should work.
	 */
	static void _makeASCII(std::basic_string<T>& string);

	// Conversion to other data types
	static BOOL	convertToBOOL(const std::basic_string<T>& string, BOOL& value);
	static BOOL	convertToU8(const std::basic_string<T>& string, U8& value);
	static BOOL	convertToS8(const std::basic_string<T>& string, S8& value);
	static BOOL	convertToS16(const std::basic_string<T>& string, S16& value);
	static BOOL	convertToU16(const std::basic_string<T>& string, U16& value);
	static BOOL	convertToU32(const std::basic_string<T>& string, U32& value);
	static BOOL	convertToS32(const std::basic_string<T>& string, S32& value);
	static BOOL	convertToF32(const std::basic_string<T>& string, F32& value);
	static BOOL	convertToF64(const std::basic_string<T>& string, F64& value);

	/////////////////////////////////////////////////////////////////////////////////////////
	// Utility functions for working with char*'s and strings

	// Like strcmp but also handles empty strings. Uses
	// current locale.
	static S32		compareStrings(const T* lhs, const T* rhs);
	static S32		compareStrings(const std::basic_string<T>& lhs, const std::basic_string<T>& rhs);
	
	// case insensitive version of above. Uses current locale on
	// Win32, and falls back to a non-locale aware comparison on
	// Linux.
	static S32		compareInsensitive(const T* lhs, const T* rhs);
	static S32		compareInsensitive(const std::basic_string<T>& lhs, const std::basic_string<T>& rhs);

	// Case sensitive comparison with good handling of numbers.  Does not use current locale.
	// a.k.a. strdictcmp()
	static S32		compareDict(const std::basic_string<T>& a, const std::basic_string<T>& b);

	// Case *in*sensitive comparison with good handling of numbers.  Does not use current locale.
	// a.k.a. strdictcmp()
	static S32		compareDictInsensitive(const std::basic_string<T>& a, const std::basic_string<T>& b);

	// Puts compareDict() in a form appropriate for LL container classes to use for sorting.
	static BOOL		precedesDict( const std::basic_string<T>& a, const std::basic_string<T>& b );

	// A replacement for strncpy.
	// If the dst buffer is dst_size bytes long or more, ensures that dst is null terminated and holds
	// up to dst_size-1 characters of src.
	static void		copy(T* dst, const T* src, size_type dst_size);
	
	// Copies src into dst at a given offset.  
	static void		copyInto(std::basic_string<T>& dst, const std::basic_string<T>& src, size_type offset);
	
#ifdef _DEBUG	
	static void		testHarness();
#endif

};

template<class T> std::basic_string<T> const LLStringUtilBase<T>::null;

typedef LLStringUtilBase<char> LLStringUtil;
typedef LLStringUtilBase<llwchar> LLWStringUtil;
typedef std::basic_string<llwchar> LLWString;

//@ Use this where we want to disallow input in the form of "foo"
//  This is used to catch places where english text is embedded in the code
//  instead of in a translatable XUI file.
class LLStringExplicit : public std::string
{
public:
	explicit LLStringExplicit(const char* s) : std::string(s) {}
	LLStringExplicit(const std::string& s) : std::string(s) {}
	LLStringExplicit(const std::string& s, size_type pos, size_type n = std::string::npos) : std::string(s, pos, n) {}
};

struct LLDictionaryLess
{
public:
	bool operator()(const std::string& a, const std::string& b)
	{
		return (LLStringUtil::precedesDict(a, b) ? true : false);
	}
};


/**
 * Simple support functions
 */

/**
 * @brief chop off the trailing characters in a string.
 *
 * This function works on bytes rather than glyphs, so this will
 * incorrectly truncate non-single byte strings.
 * Use utf8str_truncate() for utf8 strings
 * @return a copy of in string minus the trailing count characters.
 */
inline std::string chop_tail_copy(
	const std::string& in,
	std::string::size_type count)
{
	return std::string(in, 0, in.length() - count);
}

/**
 * @brief This translates a nybble stored as a hex value from 0-f back
 * to a nybble in the low order bits of the return byte.
 */
LL_COMMON_API U8 hex_as_nybble(char hex);

/**
 * @brief read the contents of a file into a string.
 *
 * Since this function has no concept of character encoding, most
 * anything you do with this method ill-advised. Please avoid.
 * @param str [out] The string which will have.
 * @param filename The full name of the file to read.
 * @return Returns true on success. If false, str is unmodified.
 */
LL_COMMON_API bool _read_file_into_string(std::string& str, const std::string& filename);

/**
 * Unicode support
 */

// Make the incoming string a utf8 string. Replaces any unknown glyph
// with the UNKOWN_CHARACTER. Once any unknown glph is found, the rest
// of the data may not be recovered.
LL_COMMON_API std::string rawstr_to_utf8(const std::string& raw);

//
// We should never use UTF16 except when communicating with Win32!
//
typedef std::basic_string<wchar_t> llutf16string;

LL_COMMON_API LLWString utf16str_to_wstring(const llutf16string &utf16str, S32 len);
LL_COMMON_API LLWString utf16str_to_wstring(const llutf16string &utf16str);

LL_COMMON_API llutf16string wstring_to_utf16str(const LLWString &utf32str, S32 len);
LL_COMMON_API llutf16string wstring_to_utf16str(const LLWString &utf32str);

LL_COMMON_API llutf16string utf8str_to_utf16str ( const std::string& utf8str, S32 len);
LL_COMMON_API llutf16string utf8str_to_utf16str ( const std::string& utf8str );

LL_COMMON_API LLWString utf8str_to_wstring(const std::string &utf8str, S32 len);
LL_COMMON_API LLWString utf8str_to_wstring(const std::string &utf8str);
// Same function, better name. JC
inline LLWString utf8string_to_wstring(const std::string& utf8_string) { return utf8str_to_wstring(utf8_string); }

//
LL_COMMON_API S32 wchar_to_utf8chars(llwchar inchar, char* outchars);

LL_COMMON_API std::string wstring_to_utf8str(const LLWString &utf32str, S32 len);
LL_COMMON_API std::string wstring_to_utf8str(const LLWString &utf32str);

LL_COMMON_API std::string utf16str_to_utf8str(const llutf16string &utf16str, S32 len);
LL_COMMON_API std::string utf16str_to_utf8str(const llutf16string &utf16str);

// Length of this UTF32 string in bytes when transformed to UTF8
LL_COMMON_API S32 wstring_utf8_length(const LLWString& wstr);

// Length in bytes of this wide char in a UTF8 string
LL_COMMON_API S32 wchar_utf8_length(const llwchar wc);

LL_COMMON_API std::string utf8str_tolower(const std::string& utf8str);

// Length in llwchar (UTF-32) of the first len units (16 bits) of the given UTF-16 string.
LL_COMMON_API S32 utf16str_wstring_length(const llutf16string &utf16str, S32 len);

// Length in utf16string (UTF-16) of wlen wchars beginning at woffset.
LL_COMMON_API S32 wstring_utf16_length(const LLWString & wstr, S32 woffset, S32 wlen);

// Length in wstring (i.e., llwchar count) of a part of a wstring specified by utf16 length (i.e., utf16 units.)
LL_COMMON_API S32 wstring_wstring_length_from_utf16_length(const LLWString & wstr, S32 woffset, S32 utf16_length, BOOL *unaligned = NULL);

/**
 * @brief Properly truncate a utf8 string to a maximum byte count.
 * 
 * The returned string may be less than max_len if the truncation
 * happens in the middle of a glyph. If max_len is longer than the
 * string passed in, the return value == utf8str.
 * @param utf8str A valid utf8 string to truncate.
 * @param max_len The maximum number of bytes in the return value.
 * @return Returns a valid utf8 string with byte count <= max_len.
 */
LL_COMMON_API std::string utf8str_truncate(const std::string& utf8str, const S32 max_len);

LL_COMMON_API std::string utf8str_trim(const std::string& utf8str);

LL_COMMON_API S32 utf8str_compare_insensitive(
	const std::string& lhs,
	const std::string& rhs);

/**
 * @brief Replace all occurences of target_char with replace_char
 *
 * @param utf8str A utf8 string to process.
 * @param target_char The wchar to be replaced
 * @param replace_char The wchar which is written on replace
 */
LL_COMMON_API std::string utf8str_substChar(
	const std::string& utf8str,
	const llwchar target_char,
	const llwchar replace_char);

LL_COMMON_API std::string utf8str_makeASCII(const std::string& utf8str);

// Hack - used for evil notecards.
LL_COMMON_API std::string mbcsstring_makeASCII(const std::string& str);

LL_COMMON_API std::string utf8str_removeCRLF(const std::string& utf8str);

LL_COMMON_API bool is_hex_string(U8* str, S32 len);


#if LL_WINDOWS
/* @name Windows string helpers
 */
//@{

/**
 * @brief Implementation the expected snprintf interface.
 *
 * If the size of the passed in buffer is not large enough to hold the string,
 * two bad things happen:
 * 1. resulting formatted string is NOT null terminated
 * 2. Depending on the platform, the return value could be a) the required
 *    size of the buffer to copy the entire formatted string or b) -1.
 *    On Windows with VS.Net 2003, it returns -1 e.g. 
 *
 * safe_snprintf always adds a NULL terminator so that the caller does not
 * need to check for return value or need to add the NULL terminator.
 * It does not, however change the return value - to let the caller know
 * that the passed in buffer size was not large enough to hold the
 * formatted string.
 *
 */

// Deal with the differeneces on Windows
namespace snprintf_hack
{
    LL_COMMON_API int snprintf(char *str, size_t size, const char *format, ...);
}

using snprintf_hack::snprintf;

/**
 * @brief Convert a wide string to std::string
 *
 * This replaces the unsafe W2A macro from ATL.
 */
LL_COMMON_API std::string ll_convert_wide_to_string(const wchar_t* in);

//@}
#endif // LL_WINDOWS

/**
 * Many of the 'strip' and 'replace' methods of LLStringUtilBase need
 * specialization to work with the signed char type.
 * Sadly, it is not possible (AFAIK) to specialize a single method of
 * a template class.
 * That stuff should go here.
 */
namespace LLStringFn
{
	/**
	 * @brief Replace all non-printable characters with replacement in
	 * string.
	 * NOTE - this will zap non-ascii
	 *
	 * @param [in,out] string the to modify. out value is the string
	 * with zero non-printable characters.
	 * @param The replacement character. use LL_UNKNOWN_CHAR if unsure.
	 */
	LL_COMMON_API void replace_nonprintable_in_ascii(
		std::basic_string<char>& string,
		char replacement);


	/**
	 * @brief Replace all non-printable characters and pipe characters
	 * with replacement in a string.
	 * NOTE - this will zap non-ascii
	 *
	 * @param [in,out] the string to modify. out value is the string
	 * with zero non-printable characters and zero pipe characters.
	 * @param The replacement character. use LL_UNKNOWN_CHAR if unsure.
	 */
	LL_COMMON_API void replace_nonprintable_and_pipe_in_ascii(std::basic_string<char>& str,
									   char replacement);


	/**
	 * @brief Remove all characters that are not allowed in XML 1.0.
	 * Returns a copy of the string with those characters removed.
	 * Works with US ASCII and UTF-8 encoded strings.  JC
	 */
	LL_COMMON_API std::string strip_invalid_xml(const std::string& input);


	/**
	 * @brief Replace all control characters (0 <= c < 0x20) with replacement in
	 * string.   This is safe for utf-8
	 *
	 * @param [in,out] string the to modify. out value is the string
	 * with zero non-printable characters.
	 * @param The replacement character. use LL_UNKNOWN_CHAR if unsure.
	 */
	LL_COMMON_API void replace_ascii_controlchars(
		std::basic_string<char>& string,
		char replacement);
}

////////////////////////////////////////////////////////////

// LLStringBase::format()
//
// This function takes a string 's' and a map 'fmt_map' of strings-to-strings.
// All occurances of strings in 's' from the left-hand side of 'fmt_map' are
// then replaced with the corresponding right-hand side of 'fmt_map', non-
// recursively.  The function returns the number of substitutions made.

// static
template<class T> 
S32 LLStringUtilBase<T>::format(std::basic_string<T>& s, const format_map_t& fmt_map)
{
	typedef typename std::basic_string<T>::size_type string_size_type_t;
	string_size_type_t scanstart = 0;
	S32 res = 0;

	// Look for the first match of any keyword, replace that keyword,
	// repeat from the end of the replacement string.  This avoids
	// accidentally performing substitution on a substituted string.
	while (1)
	{
		string_size_type_t first_match_pos = scanstart;
		string_size_type_t first_match_str_length = 0;
		std::basic_string<T> first_match_str_replacement;

		for (format_map_t::const_iterator iter = fmt_map.begin();
		     iter != fmt_map.end();
		     ++iter)
		{
			string_size_type_t n = s.find(iter->first, scanstart);
			if (n != std::basic_string<T>::npos &&
			    (n < first_match_pos ||
			     0 == first_match_str_length))
			{
				first_match_pos = n;
				first_match_str_length = iter->first.length();
				first_match_str_replacement = iter->second;
			}
		}

		if (0 == first_match_str_length)
		{
			// no more keys found to substitute from this point
			// in the string forward.
			break;
		}
		else
		{
			s.erase(first_match_pos, first_match_str_length);
			s.insert(first_match_pos, first_match_str_replacement);
			scanstart = first_match_pos +
				first_match_str_replacement.length();
			++res;
		}
	}
	return res;
}

// static
template<class T> 
S32 LLStringUtilBase<T>::compareStrings(const T* lhs, const T* rhs)
{	
	S32 result;
	if( lhs == rhs )
	{
		result = 0;
	}
	else
	if ( !lhs || !lhs[0] )
	{
		result = ((!rhs || !rhs[0]) ? 0 : 1);
	}
	else
	if ( !rhs || !rhs[0])
	{
		result = -1;
	}
	else
	{
		result = LLStringOps::collate(lhs, rhs);
	}
	return result;
}

//static 
template<class T> 
S32 LLStringUtilBase<T>::compareStrings(const std::basic_string<T>& lhs, const std::basic_string<T>& rhs)
{
	return LLStringOps::collate(lhs.c_str(), rhs.c_str());
}

// static
template<class T> 
S32 LLStringUtilBase<T>::compareInsensitive(const T* lhs, const T* rhs )
{
	S32 result;
	if( lhs == rhs )
	{
		result = 0;
	}
	else
	if ( !lhs || !lhs[0] )
	{
		result = ((!rhs || !rhs[0]) ? 0 : 1);
	}
	else
	if ( !rhs || !rhs[0] )
	{
		result = -1;
	}
	else
	{
		std::basic_string<T> lhs_string(lhs);
		std::basic_string<T> rhs_string(rhs);
		LLStringUtilBase<T>::toUpper(lhs_string);
		LLStringUtilBase<T>::toUpper(rhs_string);
		result = LLStringOps::collate(lhs_string.c_str(), rhs_string.c_str());
	}
	return result;
}

//static 
template<class T> 
S32 LLStringUtilBase<T>::compareInsensitive(const std::basic_string<T>& lhs, const std::basic_string<T>& rhs)
{
	std::basic_string<T> lhs_string(lhs);
	std::basic_string<T> rhs_string(rhs);
	LLStringUtilBase<T>::toUpper(lhs_string);
	LLStringUtilBase<T>::toUpper(rhs_string);
	return LLStringOps::collate(lhs_string.c_str(), rhs_string.c_str());
}

// Case sensitive comparison with good handling of numbers.  Does not use current locale.
// a.k.a. strdictcmp()

//static 
template<class T>
S32 LLStringUtilBase<T>::compareDict(const std::basic_string<T>& astr, const std::basic_string<T>& bstr)
{
	const T* a = astr.c_str();
	const T* b = bstr.c_str();
	T ca, cb;
	S32 ai, bi, cnt = 0;
	S32 bias = 0;

	ca = *(a++);
	cb = *(b++);
	while( ca && cb ){
		if( bias==0 ){
			if( LLStringOps::isUpper(ca) ){ ca = LLStringOps::toLower(ca); bias--; }
			if( LLStringOps::isUpper(cb) ){ cb = LLStringOps::toLower(cb); bias++; }
		}else{
			if( LLStringOps::isUpper(ca) ){ ca = LLStringOps::toLower(ca); }
			if( LLStringOps::isUpper(cb) ){ cb = LLStringOps::toLower(cb); }
		}
		if( LLStringOps::isDigit(ca) ){
			if( cnt-->0 ){
				if( cb!=ca ) break;
			}else{
				if( !LLStringOps::isDigit(cb) ) break;
				for(ai=0; LLStringOps::isDigit(a[ai]); ai++);
				for(bi=0; LLStringOps::isDigit(b[bi]); bi++);
				if( ai<bi ){ ca=0; break; }
				if( bi<ai ){ cb=0; break; }
				if( ca!=cb ) break;
				cnt = ai;
			}
		}else if( ca!=cb ){   break;
		}
		ca = *(a++);
		cb = *(b++);
	}
	if( ca==cb ) ca += bias;
	return ca-cb;
}

// static
template<class T>
S32 LLStringUtilBase<T>::compareDictInsensitive(const std::basic_string<T>& astr, const std::basic_string<T>& bstr)
{
	const T* a = astr.c_str();
	const T* b = bstr.c_str();
	T ca, cb;
	S32 ai, bi, cnt = 0;

	ca = *(a++);
	cb = *(b++);
	while( ca && cb ){
		if( LLStringOps::isUpper(ca) ){ ca = LLStringOps::toLower(ca); }
		if( LLStringOps::isUpper(cb) ){ cb = LLStringOps::toLower(cb); }
		if( LLStringOps::isDigit(ca) ){
			if( cnt-->0 ){
				if( cb!=ca ) break;
			}else{
				if( !LLStringOps::isDigit(cb) ) break;
				for(ai=0; LLStringOps::isDigit(a[ai]); ai++);
				for(bi=0; LLStringOps::isDigit(b[bi]); bi++);
				if( ai<bi ){ ca=0; break; }
				if( bi<ai ){ cb=0; break; }
				if( ca!=cb ) break;
				cnt = ai;
			}
		}else if( ca!=cb ){   break;
		}
		ca = *(a++);
		cb = *(b++);
	}
	return ca-cb;
}

// Puts compareDict() in a form appropriate for LL container classes to use for sorting.
// static 
template<class T> 
BOOL LLStringUtilBase<T>::precedesDict( const std::basic_string<T>& a, const std::basic_string<T>& b )
{
	if( a.size() && b.size() )
	{
		return (LLStringUtilBase<T>::compareDict(a.c_str(), b.c_str()) < 0);
	}
	else
	{
		return (!b.empty());
	}
}

//static
template<class T> 
void LLStringUtilBase<T>::toUpper(std::basic_string<T>& string)	
{ 
	if( !string.empty() )
	{ 
		std::transform(
			string.begin(),
			string.end(),
			string.begin(),
			(T(*)(T)) &LLStringOps::toUpper);
	}
}

//static
template<class T> 
void LLStringUtilBase<T>::toLower(std::basic_string<T>& string)
{ 
	if( !string.empty() )
	{ 
		std::transform(
			string.begin(),
			string.end(),
			string.begin(),
			(T(*)(T)) &LLStringOps::toLower);
	}
}

//static
template<class T> 
void LLStringUtilBase<T>::trimHead(std::basic_string<T>& string)
{			
	if( !string.empty() )
	{
		size_type i = 0;
		while( i < string.length() && LLStringOps::isSpace( string[i] ) )
		{
			i++;
		}
		string.erase(0, i);
	}
}

//static
template<class T> 
void LLStringUtilBase<T>::trimTail(std::basic_string<T>& string)
{			
	if( string.size() )
	{
		size_type len = string.length();
		size_type i = len;
		while( i > 0 && LLStringOps::isSpace( string[i-1] ) )
		{
			i--;
		}

		string.erase( i, len - i );
	}
}


// Replace line feeds with carriage return-line feed pairs.
//static
template<class T>
void LLStringUtilBase<T>::addCRLF(std::basic_string<T>& string)
{
	const T LF = 10;
	const T CR = 13;

	// Count the number of line feeds
	size_type count = 0;
	size_type len = string.size();
	size_type i;
	for( i = 0; i < len; i++ )
	{
		if( string[i] == LF )
		{
			count++;
		}
	}

	// Insert a carriage return before each line feed
	if( count )
	{
		size_type size = len + count;
		T *t = new T[size];
		size_type j = 0;
		for( i = 0; i < len; ++i )
		{
			if( string[i] == LF )
			{
				t[j] = CR;
				++j;
			}
			t[j] = string[i];
			++j;
		}

		string.assign(t, size);
		delete[] t;
	}
}

// Remove all carriage returns
//static
template<class T> 
void LLStringUtilBase<T>::removeCRLF(std::basic_string<T>& string)
{
	const T CR = 13;

	size_type cr_count = 0;
	size_type len = string.size();
	size_type i;
	for( i = 0; i < len - cr_count; i++ )
	{
		if( string[i+cr_count] == CR )
		{
			cr_count++;
		}

		string[i] = string[i+cr_count];
	}
	string.erase(i, cr_count);
}

//static
template<class T> 
void LLStringUtilBase<T>::replaceChar( std::basic_string<T>& string, T target, T replacement )
{
	size_type found_pos = 0;
	while( (found_pos = string.find(target, found_pos)) != std::basic_string<T>::npos ) 
	{
		string[found_pos] = replacement;
		found_pos++; // avoid infinite defeat if target == replacement
	}
}

//static
template<class T> 
void LLStringUtilBase<T>::replaceString( std::basic_string<T>& string, std::basic_string<T> target, std::basic_string<T> replacement )
{
	size_type found_pos = 0;
	while( (found_pos = string.find(target, found_pos)) != std::basic_string<T>::npos )
	{
		string.replace( found_pos, target.length(), replacement );
		found_pos += replacement.length(); // avoid infinite defeat if replacement contains target
	}
}

//static
template<class T> 
void LLStringUtilBase<T>::replaceNonstandardASCII( std::basic_string<T>& string, T replacement )
{
	const char LF = 10;
	const S8 MIN = 32;
//	const S8 MAX = 127;

	size_type len = string.size();
	for( size_type i = 0; i < len; i++ )
	{
		// No need to test MAX < mText[i] because we treat mText[i] as a signed char,
		// which has a max value of 127.
		if( ( S8(string[i]) < MIN ) && (string[i] != LF) )
		{
			string[i] = replacement;
		}
	}
}

//static
template<class T> 
void LLStringUtilBase<T>::replaceTabsWithSpaces( std::basic_string<T>& str, size_type spaces_per_tab )
{
	const T TAB = '\t';
	const T SPACE = ' ';

	std::basic_string<T> out_str;
	// Replace tabs with spaces
	for (size_type i = 0; i < str.length(); i++)
	{
		if (str[i] == TAB)
		{
			for (size_type j = 0; j < spaces_per_tab; j++)
				out_str += SPACE;
		}
		else
		{
			out_str += str[i];
		}
	}
	str = out_str;
}

//static
template<class T> 
BOOL LLStringUtilBase<T>::containsNonprintable(const std::basic_string<T>& string)
{
	const char MIN = 32;
	BOOL rv = FALSE;
	for (size_type i = 0; i < string.size(); i++)
	{
		if(string[i] < MIN)
		{
			rv = TRUE;
			break;
		}
	}
	return rv;
}

//static
template<class T> 
void LLStringUtilBase<T>::stripNonprintable(std::basic_string<T>& string)
{
	const char MIN = 32;
	size_type j = 0;
	if (string.empty())
	{
		return;
	}
	char* c_string = new char[string.size() + 1];
	if(c_string == NULL)
	{
		return;
	}
	strcpy(c_string, string.c_str());	/*Flawfinder: ignore*/
	char* write_head = &c_string[0];
	for (size_type i = 0; i < string.size(); i++)
	{
		char* read_head = &string[i];
		write_head = &c_string[j];
		if(!(*read_head < MIN))
		{
			*write_head = *read_head;
			++j;
		}
	}
	c_string[j]= '\0';
	string = c_string;
	delete []c_string;
}

template<class T> 
void LLStringUtilBase<T>::_makeASCII(std::basic_string<T>& string)
{
	// Replace non-ASCII chars with LL_UNKNOWN_CHAR
	for (size_type i = 0; i < string.length(); i++)
	{
		if (string[i] > 0x7f)
		{
			string[i] = LL_UNKNOWN_CHAR;
		}
	}
}

// static
template<class T> 
void LLStringUtilBase<T>::copy( T* dst, const T* src, size_type dst_size )
{
	if( dst_size > 0 )
	{
		size_type min_len = 0;
		if( src )
		{
			min_len = llmin( dst_size - 1, strlen( src ) );  /* Flawfinder: ignore */
			memcpy(dst, src, min_len * sizeof(T));		/* Flawfinder: ignore */
		}
		dst[min_len] = '\0';
	}
}

// static
template<class T> 
void LLStringUtilBase<T>::copyInto(std::basic_string<T>& dst, const std::basic_string<T>& src, size_type offset)
{
	if ( offset == dst.length() )
	{
		// special case - append to end of string and avoid expensive
		// (when strings are large) string manipulations
		dst += src;
	}
	else
	{
		std::basic_string<T> tail = dst.substr(offset);

		dst = dst.substr(0, offset);
		dst += src;
		dst += tail;
	};
}

// True if this is the head of s.
//static
template<class T> 
BOOL LLStringUtilBase<T>::isHead( const std::basic_string<T>& string, const T* s ) 
{ 
	if( string.empty() )
	{
		// Early exit
		return FALSE;
	}
	else
	{
		return (strncmp( s, string.c_str(), string.size() ) == 0);
	}
}

template<class T> 
BOOL LLStringUtilBase<T>::convertToBOOL(const std::basic_string<T>& string, BOOL& value)
{
	if( string.empty() )
	{
		return FALSE;
	}

	std::basic_string<T> temp( string );
	trim(temp);
	if( 
		(temp == "1") || 
		(temp == "T") || 
		(temp == "t") || 
		(temp == "TRUE") || 
		(temp == "true") || 
		(temp == "True") )
	{
		value = TRUE;
		return TRUE;
	}
	else
	if( 
		(temp == "0") || 
		(temp == "F") || 
		(temp == "f") || 
		(temp == "FALSE") || 
		(temp == "false") || 
		(temp == "False") )
	{
		value = FALSE;
		return TRUE;
	}

	return FALSE;
}

template<class T> 
BOOL LLStringUtilBase<T>::convertToU8(const std::basic_string<T>& string, U8& value) 
{
	S32 value32 = 0;
	BOOL success = convertToS32(string, value32);
	if( success && (U8_MIN <= value32) && (value32 <= U8_MAX) )
	{
		value = (U8) value32;
		return TRUE;
	}
	return FALSE;
}

template<class T> 
BOOL LLStringUtilBase<T>::convertToS8(const std::basic_string<T>& string, S8& value) 
{
	S32 value32 = 0;
	BOOL success = convertToS32(string, value32);
	if( success && (S8_MIN <= value32) && (value32 <= S8_MAX) )
	{
		value = (S8) value32;
		return TRUE;
	}
	return FALSE;
}

template<class T> 
BOOL LLStringUtilBase<T>::convertToS16(const std::basic_string<T>& string, S16& value) 
{
	S32 value32 = 0;
	BOOL success = convertToS32(string, value32);
	if( success && (S16_MIN <= value32) && (value32 <= S16_MAX) )
	{
		value = (S16) value32;
		return TRUE;
	}
	return FALSE;
}

template<class T> 
BOOL LLStringUtilBase<T>::convertToU16(const std::basic_string<T>& string, U16& value) 
{
	S32 value32 = 0;
	BOOL success = convertToS32(string, value32);
	if( success && (U16_MIN <= value32) && (value32 <= U16_MAX) )
	{
		value = (U16) value32;
		return TRUE;
	}
	return FALSE;
}

template<class T> 
BOOL LLStringUtilBase<T>::convertToU32(const std::basic_string<T>& string, U32& value) 
{
	if( string.empty() )
	{
		return FALSE;
	}

	std::basic_string<T> temp( string );
	trim(temp);
	U32 v;
	std::basic_istringstream<T> i_stream((std::basic_string<T>)temp);
	if(i_stream >> v)
	{
		value = v;
		return TRUE;
	}
	return FALSE;
}

template<class T> 
BOOL LLStringUtilBase<T>::convertToS32(const std::basic_string<T>& string, S32& value) 
{
	if( string.empty() )
	{
		return FALSE;
	}

	std::basic_string<T> temp( string );
	trim(temp);
	S32 v;
	std::basic_istringstream<T> i_stream((std::basic_string<T>)temp);
	if(i_stream >> v)
	{
		//TODO: figure out overflow and underflow reporting here
		//if((LONG_MAX == v) || (LONG_MIN == v))
		//{
		//	// Underflow or overflow
		//	return FALSE;
		//}

		value = v;
		return TRUE;
	}
	return FALSE;
}

template<class T> 
BOOL LLStringUtilBase<T>::convertToF32(const std::basic_string<T>& string, F32& value) 
{
	F64 value64 = 0.0;
	BOOL success = convertToF64(string, value64);
	if( success && (-F32_MAX <= value64) && (value64 <= F32_MAX) )
	{
		value = (F32) value64;
		return TRUE;
	}
	return FALSE;
}

template<class T> 
BOOL LLStringUtilBase<T>::convertToF64(const std::basic_string<T>& string, F64& value)
{
	if( string.empty() )
	{
		return FALSE;
	}

	std::basic_string<T> temp( string );
	trim(temp);
	F64 v;
	std::basic_istringstream<T> i_stream((std::basic_string<T>)temp);
	if(i_stream >> v)
	{
		//TODO: figure out overflow and underflow reporting here
		//if( ((-HUGE_VAL == v) || (HUGE_VAL == v))) )
		//{
		//	// Underflow or overflow
		//	return FALSE;
		//}

		value = v;
		return TRUE;
	}
	return FALSE;
}

template<class T> 
void LLStringUtilBase<T>::truncate(std::basic_string<T>& string, size_type count)
{
	size_type cur_size = string.size();
	string.resize(count < cur_size ? count : cur_size);
}

#endif  // LL_STRING_H
