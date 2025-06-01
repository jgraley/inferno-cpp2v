#include "move_in_pass.hpp"
#include "db/x_tree_database.hpp"
#include "scaffold_ops.hpp"

#include <iostream>

using namespace SR;

MoveInPass::MoveInPass( XTreeDatabase *db_, ScaffoldOps *sops_ ) :
    db( db_ ),
    sops( sops_ )
{
}


void MoveInPass::Run(MovesMap &moves_map)
{	
	INDENT("Z");
	TRACE("Got %u map entries\n", moves_map.mm.size());
	
	for( auto &p : moves_map.mm )
	{
		// Hopefully our "to" scaffold node made it through inversion and is now in the main tree. 
		// Build a TZ around it. Also get the actual moving content which is in an extra tree.
		XTreeZone inverted_main_tree_zone = sops->TreeZoneAroundScaffoldNode( p.first, DBCommon::TreeOrdinal::MAIN ).GetXTreeZone();	        
		XTreeZone extra_tree_zone = p.second;
		
		// Swap the moving content in and the scaffold out
		db->SwapTreeToTree( extra_tree_zone, vector<TreeZone *>(),
							inverted_main_tree_zone, vector<TreeZone *>() );
							
		// We're done with the extra tree zone which now contains scaffold
		db->TeardownTree(extra_tree_zone.GetTreeOrdinal()); 				                                     	
	}
}

