/**
 *
 * Copyright (c) 2009-2010, Kitty Barnett
 *
 * The source code in this file is provided to you under the terms of the
 * GNU General Public License, version 2.0, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. Terms of the GPL can be found in doc/GPL-license.txt
 * in this distribution, or online at http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * By copying, modifying or distributing this software, you acknowledge that
 * you have read and understood your obligations described above, and agree to
 * abide by those obligations.
 *
 */

#include "llviewerprecompiledheaders.h"

#include "rlvmultistringsearch.h"

// ====================================================================================

#ifndef RLV_LOWORD
	#define RLV_LOWORD(x)		( (U16)( ((U32)x) & 0xFFFF) )
#endif // RLV_LOWORD

#ifndef RLV_HIWORD
	#define RLV_HIWORD(x)		( (U16)( (((U32)x) >> 16) & 0xFFFF) )
#endif // RLV_HIWORD

// ====================================================================================

// (TODO-RLV: oops, forgot I was experimenting with word matching, get rid of that again?)
#define isLetter(ch) \
	( ( (ch >= 'a') && (ch <= 'z') ) || ( (ch >= 'A') && (ch <= 'Z') ) )

RlvMultiStringSearch::RlvMultiStringSearch()
	: m_FSM(256),	// Start our FSM with room for 256 states (enough for all attachment point names)
	  m_cntState(0)
{
}

void RlvMultiStringSearch::addKeyword(const std::string& strKeyword, U16 nParam) {
	U16 nCurState = 0;	// Always start the loop at state 0

	//
	// Make sure there are enough unused rows to accomodate the worst case (== strKeyword.length() new states)
	//
	size_t nMaxState = m_FSM.getSize();
	if (m_cntState + strKeyword.length() > nMaxState)
		// Allocate enough new rows (in batches of 256 rows)
		m_FSM.resize(nMaxState + ((strKeyword.length() / 256) + 1) * 256);

	//
	// Walk the string character by character
	//
	for (int idxCh = 0, cntCh = strKeyword.length(); idxCh < cntCh; idxCh++) {
		// Look up the next state for current character
		unsigned char ch = strKeyword[idxCh];
		U16 nState = RLV_LOWORD(m_FSM[nCurState][ch]);

		// If we're at the last character in the keyword then set the termination bit
		if (cntCh - 1 == idxCh)
		{
			// (Only set the termination bit for the state because this keyword might be a substring of another keyword)
			m_FSM[nCurState][ch] = (nParam << 16) | (nState | 0x8000);
		}
		else if ( (nState & 0x7FFF) == 0 ) // If the new state is 0 then we're creating a new path
		{
			// (Preserve the termination bit because another keyword might be a substring of this one)
			nState = ++m_cntState | (nState & 0x8000);

			// Store the new path in the FSM
			//m_FSM[nCurState][ch] = (nParam << 16) | nState;
			m_FSM[nCurState][ch] |= nState;
		}

		nCurState = nState & 0x7FFF; // Mask out the termination bit since we never need it for the current state
	}
}

// (Iterating over a "const char*" is *significantly* faster than "std::string")
bool RlvMultiStringSearch::findNext(const char* pstrText, int idxCh, int cntCh, RlvMultiStringSearchMatch& match, bool fWordMatch) const
{
	U16 nCurState = 0;		// Always start the loop at state 0
	U32 nLastMatch = 0;		// Holds the state of the last (possibly partial) keyword match

	//
	// Walk the string character by character
	//
	for (; idxCh < cntCh; idxCh++)
	{
		// Keep track of the current state in case we need to backtrack
		U16 nPrevState = nCurState;

		// If we're currently in state 0, save the current character index (for backtracking or as keyword index match)
		if (nCurState == 0)
			match.idxMatch = idxCh;

		// Look up the current character in the FSM
		unsigned char ch = (unsigned char)pstrText[idxCh];
		U32 nCell = m_FSM[nCurState & 0x7FFF][ch];

		// If the termination bit is set then we found a keyword substring match
		// If the next state is non-zero then we can't stop yet because the matched keyword might be a substring of another keyword
		if (nCell & 0x8000)
		{
			if ( 0 == (nCell & 0x7FFF) )
			{
				// Termination bit with 'next state' equal to 0: matched keyword which isn't also a substring of any other keyword
				match.lenMatch = idxCh - match.idxMatch + 1;
				match.nParam = RLV_HIWORD(nCell);

				// Rudimentary word matching: check if the match is a 'word'
				if
					( 
						(!fWordMatch) ||
						(
							( (0 == match.idxMatch) || (!isLetter(pstrText[match.idxMatch - 1])) ) && // Start of string OR non-letter
							( (!isLetter(pstrText[match.idxMatch + match.lenMatch])) )
						)
					)
				{
					return true;
				}
				
				// Not a word, but there's no need to backtrack: we can move on from the character after the current one
				nCell = 0;				// Will set nCurState == 0 further down
				match.idxMatch = idxCh; // Makes sure we move on to the next character instead of backtracking
			}
			else
			{
				nLastMatch = nCell;

				// In case it turns out that we need to backtrack and return this match, save the length of this match
				match.lenMatch = idxCh - match.idxMatch + 1;
			}
		}

		nCurState = RLV_LOWORD(nCell);

		// If our new state is 0, but our previous state wasn't, then we followed a false lead and need to backtrack
		if ( (nPrevState != 0) && (nCurState == 0) )
		{
			//   * if nLastMatch == 0 then we need to backtrack and keep going
			//   * if nLastMatch != 0 then we previously encountered a keyword match so return that one
			if (nLastMatch) {
				// Rudimentary word matching: check if the match is a 'word'
				if
					( 
						(!fWordMatch) ||
						(
							( (0 == match.idxMatch) || (!isLetter(pstrText[match.idxMatch - 1])) ) && // Start of string OR non-letter
							( (!isLetter(pstrText[match.idxMatch + match.lenMatch])) )
						)
					)
				{
					match.nParam = RLV_HIWORD(nLastMatch);
					return true;
				} else
					// Not a word match, so throw away this partial match and backtrack
					nLastMatch = 0;
			}

			idxCh = match.idxMatch;
		}
	}

	// We encountered a match, but while investigating whether it was a substring of another keyword we ran out of characters
	if (nLastMatch)
	{
		// Rudimentary word matching: check if we started at the beginning of a word (we know the one behind us is '\0')
		if ( (!fWordMatch) || ( (0 == match.idxMatch) || (!isLetter(pstrText[match.idxMatch - 1])) ) )
		{
			match.nParam = RLV_HIWORD(nLastMatch);
			return true;
		}
	}

	// None of the keywords is contained in the string: return failure
	match.idxMatch = -1;
	return false;
}

bool RlvMultiStringSearch::findLast(const std::string& strText, RlvMultiStringSearchMatch& match) const {
	RlvMultiStringSearchMatch matchTemp;
	match.idxMatch = -1;	// (Needed to make the return work in case we don't find anything)
	matchTemp.lenMatch = 0; // (Needed to make the first loop iteration start at 0)
	
	// Iterating over a "const char*" is *significantly* faster than "std::string"
	const char* pstrText = strText.c_str();
	int lenText = strText.length();

	while (findNext(pstrText, matchTemp.idxMatch + matchTemp.lenMatch + 1, lenText, matchTemp))
		match = matchTemp;

	return (match.idxMatch != -1);
}

std::vector<RlvMultiStringSearchMatch> RlvMultiStringSearch::findAll(const std::string& strText) {
	std::vector<RlvMultiStringSearchMatch> arMatch;

	RlvMultiStringSearchMatch match;
	match.lenMatch = 0; // (Needed to make the first loop iteration start at 0)
	
	while (findNext(strText, match.idxMatch + match.lenMatch + 1, match))
		arMatch.push_back(match);

	return arMatch;
}

// ====================================================================================
