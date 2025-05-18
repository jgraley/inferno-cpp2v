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
		const NodeTable::Row &scaffold_row = db->GetNodeRow(scaffold_node);
		ASSERT( scaffold_row.incoming_xlinks.size() == 1 );
		XLink scaffold_base_xlink = SoloElementOf(scaffold_row.incoming_xlinks);
		unique_ptr<XTreeZone> scaffold_tree_zone = XTreeZone::CreateFromScaffold( scaffold_base_xlink ); 
		unique_ptr<MutableTreeZone> scaffold_mutable_tree_zone = db->CreateMutableTreeZone( scaffold_tree_zone->GetBaseXLink(),
													                                     	scaffold_tree_zone->GetTerminusXLinks() );		
        // TODO Down to here in a helper fn, but some of it is also elsewhere search for CreateFromScaffold
        
        MovesMap::MovePayload &mp = p.second;        										                                     	
		DBCommon::TreeOrdinal moving_tree_ordinal = mp.first;
		MutableTreeZone &moving_zone = mp.second;
		TRACE("Exchanging: ")(*scaffold_mutable_tree_zone)("\n moving tree ordinal: \n")(moving_zone)(" in tree #%u\n", moving_tree_ordinal);
		db->SwapTreeToTree( moving_tree_ordinal, moving_zone, vector<MutableTreeZone *>(),
							DBCommon::TreeOrdinal::MAIN, *scaffold_mutable_tree_zone, vector<MutableTreeZone *>() );
		db->TeardownTree(moving_tree_ordinal); // Don't leak it									                                     	
	}
}

