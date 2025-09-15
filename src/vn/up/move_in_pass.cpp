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


void MoveInPass::Run(MovesMap &moves_map, ReplaceAssignments *assignments)
{	
	INDENT("Z");
	TRACE("Got %u map entries\n", moves_map.mm.size());
	set<DBCommon::TreeOrdinal> ordinals_to_tear_down;
	
	for( auto &p : moves_map.mm )
	{
		// Hopefully our "to" scaffold node made it through inversion and is now in the main tree. 
		// Build a TZ around it. Also get the actual moving content which is in an extra tree.
		TreeZone main_tree_zone = sops->TreeZoneAroundScaffoldNode( p.first, db->GetMainTreeOrdinal() );	        
		TreeZone extra_tree_zone = p.second.zone;
		
		// Notice if some of the assignments go to XLinks that will get
		// damaged by the swap
		vector<Originators> fixups( main_tree_zone.GetNumTerminii() ); // one per terminus
		for( size_t i=0; i<fixups.size(); i++ )
		{
			for( auto p : *assignments )
				if( p.second == main_tree_zone.GetTerminusXLink(i) )
					fixups[i].insert( p.first );
		}
				
		// Swap the moving content in and the scaffold out
		db->SwapTreeToTree( extra_tree_zone, vector<TreeZone *>(),
							main_tree_zone, vector<TreeZone *>() );
		ordinals_to_tear_down.insert( extra_tree_zone.GetTreeOrdinal() );
		
		// Fix up XLinks damaged by the swap.
		for( size_t i=0; i<fixups.size(); i++ )
		{
			for( PatternLink plink : fixups[i] )
				assignments->at(plink) = main_tree_zone.GetTerminusXLink(i);
		}

		// Capture the replace assignments
		for( PatternLink plink : p.second.originators )
		{
			string tn = main_tree_zone.GetBaseNode()->GetTypeName();
			ASSERT( tn.find("Scaffold") == string::npos )(tn);
			assignments->insert( make_pair(plink, main_tree_zone.GetBaseXLink()) );  				
		}
	}

	// XLink memory safety: discard tree zones in the moves map before 
	// tearing down tree, which will delete the underlying TreePtr<>		
	moves_map.mm.clear();

	// We're done with the extra trees which now contains scaffold
	for( DBCommon::TreeOrdinal ord : ordinals_to_tear_down )		
		db->TeardownTree(ord); 
}

