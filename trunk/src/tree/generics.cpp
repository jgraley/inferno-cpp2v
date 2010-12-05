/*
 * generics.cpp
 *
 *  Created on: 14 Jun 2009
 *      Author: jgraley
 */

#include "tree.hpp"

struct MyNode : Node
{
	NODE_FUNCTIONS
	Sequence<Node> s;
};

void GenericsTest()
{
	TRACE();
    shared_ptr<MyNode> n( new MyNode );
	TRACE();
    vector< Itemiser::Element * > i = n->Itemise();
	TRACE();
    for( int e=0; e<i.size(); e++ )
        TRACE("element at %p\n", i[e]);
    ASSERT( i.size() == 1 )("%d elements\n", i.size());
}

