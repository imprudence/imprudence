/*
 *  LLCalcParser.cpp
 *  SecondLife
 *
 *  Created by Aimee Walton on 28/09/2008.
 *  Copyright 2008 Aimee Walton.
 *
 */

#include "linden_common.h"

#include "llcalcparser.h"

#include <boost/version.hpp>
#if BOOST_VERSION >= 103600
using namespace boost::spirit::classic;
#else
using namespace boost::spirit;
#endif

F32 LLCalcParser::lookup(const std::string::iterator& start, const std::string::iterator& end) const
{
	LLCalc::calc_map_t::iterator iter;

	std::string name(start, end);
	
	if (mConstants)
	{
		iter = mConstants->find(name);
		if (iter != mConstants->end())
		{
			return (*iter).second;
		}
	}
	else
	{
		// This should never happen!
		throw_(end, std::string("Missing constants table"));
	}
	
	if (mVariables)
	{
		iter = mVariables->find(name);
		if (iter != mVariables->end())
		{
			return (*iter).second;
		}
	}
	
	throw_(end, std::string("Unknown symbol " + name));
	return 0.f;
}
