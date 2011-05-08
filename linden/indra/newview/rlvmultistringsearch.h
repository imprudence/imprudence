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

#ifndef RLV_MULTISTRINGSEARCH_H
#define RLV_MULTISTRINGSEARCH_H

// ============================================================================
// Template classes for our state machine (2 dimensional array of type T)

// STL vector
template<typename T> class RlvMultiStringSearchFSM_STL
{
public:
	/*
	 * Constructor/destructor
	 */

	// Initialize the FSM with an initial capacity of 'nCapacity' states
	RlvMultiStringSearchFSM_STL(size_t nCapacity)
	{
		m_arFSM.reserve(nCapacity);

		T* pT;
		for (size_t idx = 0; idx < nCapacity; idx++)
		{
			// The width of each row is determined by the alphabet we're using (in this case UTF-8
			// so while every character might consist of multiple bytes there are
			// still only 256 'columns' in the state machine)
			pT = new T[256]();

			// The above *should* initialize to 0 but since we can't account for every compiler doing it :(
			memset(pT, 0, sizeof(T) * 256);

			m_arFSM.push_back(pT);
		}
	};

	~RlvMultiStringSearchFSM_STL()
	{
		// Free any memory we previously allocated
		for (int idx = 0, cnt = m_arFSM.size(); idx < cnt; idx++)
			delete[] m_arFSM[idx];
	}

	/*
	 * Operators
	 */
	// ASSERTION: nState < m_arFSM.size() at all times
	// In other words: do *NOT* go out of bounds on the array (no memory will have allocated for that non-existing state)
	// (There probably should be a check for that even in release but it seems wasteful, just don't do it :p)
	inline T* operator[](size_t nState) 
	{
		//#ifdef _DEBUG
		//	assert( nState < m_arFSM.size() );
		//#endif // _DEBUG

		return m_arFSM[nState]; 
	}
	inline const T* operator[](size_t nState) const
	{
		//#ifdef _DEBUG
		//	assert( nState < m_arFSM.size() );
		//#endif // _DEBUG

		return m_arFSM[nState]; 
	}

	/*
	 * Public member functions
	 */

	size_t getSize() const { return m_arFSM.size(); }

	void resize(size_t nNewCapacity)
	{
		// Get our current capacity (only rows > capacity need memory allocated)
		size_t nCurCapacity = m_arFSM.capacity();

		// Only expand, never shrink
		if (nNewCapacity <= nCurCapacity)
		{
			//#ifdef _DEBUG
			//	assert(false);
			//#endif //_DEBUG

			return;
		}
		m_arFSM.resize(nNewCapacity);

		// For each new state we added, allocate memory for the columns
		for(size_t idx = nCurCapacity; idx < nNewCapacity; idx++)
			// The memset is redundant (or rather *should* be) but since we can't account for every compiler doing it :(
			m_arFSM[idx] = (T*)memset(new T[256](), 0, sizeof(T) * 256);
	}

protected:
	/*
	 * Member variables
	 */
	std::vector<T*> m_arFSM;
};

// ============================================================================

struct RlvMultiStringSearchMatch
{
	int idxMatch;	// Starting character index into the string of the matched keyword (-1 if no match)
	int lenMatch;	// Length of the matched keyword (undefined if no match)
	U16 nParam;		// User supplied parameter for the matched keyword (undefined if no match)

	RlvMultiStringSearchMatch() : idxMatch(-1) {}
};

// ============================================================================
// The actual search class

class RlvMultiStringSearch 
{
public:
	/*
	 * Constructor/destructor
	 */
	RlvMultiStringSearch();
	//~RlvMultiStringSearch();

	/*
	 * Public member functions
	 */

	// Add a keyword to the state machine (if it already exists then it will simply overwrite the existing parameter)
	void addKeyword(const std::string& strKeyword, U16 nParam);

	BOOL getExactMatchParam(const std::string& strText, U16& nParam) const
	{
		RlvMultiStringSearchMatch match;
		if (findFirst(strText, match))
		{
			// We have an exact match if the starting index is 0
			// and the length of the match matches the length of the string
			if ( (0 == match.idxMatch) && (match.lenMatch == (int)strText.length()) )
			{
				nParam = match.nParam;
				return TRUE;
			}
		}

		return FALSE;	// Fall-through: no (exact) match
	}

	// Finds the first occurance of any keyword in the supplied string
	bool findFirst(const std::string& strText, RlvMultiStringSearchMatch& match) const;
	// Finds the next occurance of any keyword in the supplied string
	bool findNext(const std::string& strText, int idxCh, RlvMultiStringSearchMatch& match) const;
	// Finds all occurances of any keyword in the supplied string
	std::vector<RlvMultiStringSearchMatch> findAll(const std::string& strText);
	// Finds the last occurance of any keyword in the supplied string (non-optimized)
	bool findLast(const std::string& strText, RlvMultiStringSearchMatch& match) const;

protected:
	// Finds the next occurance of any keyword in the supplied string
	bool findNext(const char* pstrText, int idxCh, int cntCh, RlvMultiStringSearchMatch& match, bool fWordMatch = true) const;

	/*
	 * Member variables
	 */
	RlvMultiStringSearchFSM_STL<U32> m_FSM;	// Our finite state machine (4 bytes * 256 = 1Kb of memory/state)
											// HIWORD(U32) = 16-bits of user data
											// LOWORD(U32) = ABBBBBBBBBBBBBBB
											//		A = termination bit
											//			If (set) and (B == 0): match
											//			If (set) and (B != 0): match, but might only be a substring of another keyword
											//		B = next state (0..32767)
											//			If (B == 0): false lead -> backtrack
											//			If (B != 0): partial keyword match, next state
	size_t	m_cntState;						// The number of states in the FSM (= the number of *used* rows in the array)
};

// ============================================================================
// Inlined member functions
//

inline bool RlvMultiStringSearch::findFirst(const std::string& strText, RlvMultiStringSearchMatch& match) const
{
	return findNext(strText.c_str(), 0, strText.length(), match);
}

inline bool RlvMultiStringSearch::findNext(const std::string& strText, int idxCh, RlvMultiStringSearchMatch& match) const
{
	return findNext(strText.c_str(), idxCh, strText.length(), match);
}

// ============================================================================

#endif // RLV_MULTISTRINGSEARCH_H
