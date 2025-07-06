#include "move_in_pass.hpp"
#include "db/x_tree_database.hpp"
#include "scaffold_ops.hpp"
#include "up_common.hpp"

#include <iostream>

using namespace SR;

MoveInPass::MoveInPass( XTreeDatabase *db_, ScaffoldOps *sops_ ) :
    db( db_ ),
    sops( sops_ )
{
}


Assignments MoveInPass::Run(MovesMap &moves_map)
{	
	INDENT("Z");
	TRACE("Got %u map entries\n", moves_map.mm.size());
	Assignments assignments;
	
	for( auto &p : moves_map.mm )
	{
		// Hopefully our "to" scaffold node made it through inversion and is now in the main tree. 
		// Build a TZ around it. Also get the actual moving content which is in an extra tree.
		TreeZone main_tree_zone = sops->TreeZoneAroundScaffoldNode( p.first, db->GetMainTreeOrdinal() );	        
		TreeZone extra_tree_zone = p.second.zone;
		
		// Swap the moving content in and the scaffold out
		db->SwapTreeToTree( extra_tree_zone, vector<TreeZone *>(),
							main_tree_zone, vector<TreeZone *>() );
							
		// We're done with the extra tree zone which now contains scaffold
		db->TeardownTree(extra_tree_zone.GetTreeOrdinal()); 	
		
		// Capture the replace assignments
		for( PatternLink plink : p.second.originators	 )
			assignments.insert( make_pair(plink, make_pair(main_tree_zone.GetBaseNode(), main_tree_zone.GetBaseXLink())) );		                                     	
	}
	
	return assignments;
}

