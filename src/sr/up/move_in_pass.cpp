#include "move_in_pass.hpp"

#include <iostream>

using namespace SR;

MoveInPass::MoveInPass( XTreeDatabase *db_ ) :
    db( db_ )
{
}


void MoveInPass::Run(MovesMap &moves_map)
{	
	INDENT("Z");
	
#ifdef NEW_THING
	for( auto p : moves_map.mm )
	{
		TreePtr<Node> scaffold_node = p.first;
		unique_ptr<FreeZone> move_zone = move(p.second);
		
		const NodeTable::Row &scaffold_row = db->GetNodeRow(scaffold_node);
		ASSERT( scaffold_row.incoming_xlinks.size() == 1 );
		XLink scaffold_base_xlink = SoloElementOf(scaffold_row.incoming_xlinks);
		unique_ptr<XTreeZone> scaffold_tree_zone = CreateFromScaffold( XLink scaffold_xlink );
		unique_ptr<MutableTreeZone> scaffold_mutable_tree_zone = db->CreateMutableTreeZone( scaffold_tree_zone->GetBaseXLink(),
													                                     	scaffold_tree_zone->GetTerminusXLinks() );
		db->MainTreeExchange( scaffold_mutable_tree_zone, move_zone, vector<MutableTreeZone *>(), false );											                                     	
	}
#endif
}

