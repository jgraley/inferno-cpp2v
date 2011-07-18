/*
 * for_to_while.hpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#ifndef FOR_TO_WHILE_HPP
#define FOR_TO_WHILE_HPP

#include "sr/search_replace.hpp"

namespace Steps {

// Convert for loops into while loops, preserving correct 
// behaviour of continue, which always means "jump to the 
// bottom of the body"
class ForToWhile : public SearchReplace
{
public:
	ForToWhile();
};

// Convert While loops to Do loops
class WhileToDo : public SearchReplace
{
public:
	WhileToDo();
};

}; // end namespace

#endif 

