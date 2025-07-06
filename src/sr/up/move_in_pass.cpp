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


void MoveInPass::Run(MovesMap &moves_map, Assignments *assignments)
{	
	INDENT("Z");
	TRACE("Got %u map entries\n", moves_map.mm.size());
	
	for( auto &p : moves_map.mm )
	{
		// Hopefully our "to" scaffold node made it through inversion and is now in the main tree. 
		// Build a TZ around it. Also get the actual moving content which is in an extra tree.
		TreeZone main_tree_zone = sops->TreeZoneAroundScaffoldNode( p.first, db->GetMainTreeOrdinal() );	        
		TreeZone extra_tree_zone = p.second.zone;
		
		// Notice if some of the assignments go to XLinks that will get
		// damaged by the swap
		list<Originators> v; // one per terminus
		for( XLink xlink : main_tree_zone.GetTerminusXLinks() )
		{
			v.push_back({});
			for( auto p : *assignments )
				if( p.second.second == xlink )
					v.back().insert( p.first );
		}
				
		// Swap the moving content in and the scaffold out
		db->SwapTreeToTree( extra_tree_zone, vector<TreeZone *>(),
							main_tree_zone, vector<TreeZone *>() );
				
		// Fix up XLinks damaged by the swap.
		for( XLink xlink : main_tree_zone.GetTerminusXLinks() )
		{
			// v.front() has set of originator plinks
			// x is noew correct xlink
			for( PatternLink plink : v.front() )
				assignments->at(plink).second = xlink;
			v.pop_front();
		}

		// We're done with the extra tree zone which now contains scaffold
		db->TeardownTree(extra_tree_zone.GetTreeOrdinal()); 	
		
		// Capture the replace assignments
		for( PatternLink plink : p.second.originators )
			assignments->insert( make_pair(plink, make_pair(main_tree_zone.GetBaseNode(), main_tree_zone.GetBaseXLink())) );		                                     	
	}
}

