/*
 * slave_test
 */

#ifndef SLAVE_TEST_HPP
#define SLAVE_TEST_HPP

#include "helpers/search_replace.hpp"

class SlaveTest : public InPlaceTransformation
{
public:
	using Transformation::operator();
	virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

#endif
