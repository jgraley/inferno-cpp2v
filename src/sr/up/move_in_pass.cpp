#include "move_in_pass.hpp"

#include <iostream>

using namespace SR;

MoveInPass::MoveInPass( XTreeDatabase *db_ ) :
    db( db_ ),
    dfr( db ),
    boundaries( dfr )
{
}


void MoveInPass::Run(shared_ptr<Patch> &layout, MovesMap &moves_map)
{	
	INDENT("M");
}

