#include "move_in_pass.hpp"
#include "db/x_tree_database.hpp"

#include <iostream>

using namespace SR;

MoveInPass::MoveInPass( XTreeDatabase *db_ ) :
    db( db_ )
{
}


void MoveInPass::Run(MovesMap &moves_map)
{	
	INDENT("Z");
	TRACE("Got %u map entries\n", moves_map.mm.size());
	
	for( auto &p : moves_map.mm )
	{
		TreePtr<Node> scaffold_node = p.first;
		unique_ptr<FreeZone> move_zone = move(p.second);
			
		TRACE("Got map entry, scaffold node: ")(*scaffold_node)("\n free zone: \n")(*move_zone)("\n");
		const NodeTable::Row &scaffold_row = db->GetNodeRow(scaffold_node);
		ASSERT( scaffold_row.incoming_xlinks.size() == 1 );
		XLink scaffold_base_xlink = SoloElementOf(scaffold_row.incoming_xlinks);
		unique_ptr<XTreeZone> scaffold_tree_zone = XTreeZone::CreateFromScaffold( scaffold_base_xlink );
		unique_ptr<MutableTreeZone> scaffold_mutable_tree_zone = db->CreateMutableTreeZone( scaffold_tree_zone->GetBaseXLink(),
													                                     	scaffold_tree_zone->GetTerminusXLinks() );
		TRACE("Exchanging: ")(*scaffold_mutable_tree_zone)("\n free zone: \n")(*move_zone)("\n");
		db->MainTreeExchange( scaffold_mutable_tree_zone.get(), move_zone.get(), vector<MutableTreeZone *>(), false );											                                     	
	}
}

