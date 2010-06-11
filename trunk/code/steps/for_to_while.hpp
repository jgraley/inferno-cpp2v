/*
 * for_to_while.hpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#ifndef FOR_TO_WHILE_HPP
#define FOR_TO_WHILE_HPP

#include "helpers/search_replace.hpp"

class ForToWhile : public Transformation
{
public:
	virtual void operator()( SharedPtr<Node> context, SharedPtr<Node> *proot );
};

#endif 
