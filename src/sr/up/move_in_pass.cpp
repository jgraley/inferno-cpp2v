#include "move_in_pass.hpp"
#include "db/x_tree_database.hpp"
#include "update_ops.hpp"

#include <iostream>

using namespace SR;

MoveInPass::MoveInPass( XTreeDatabase *db_, UpdateOps *ups_ ) :
    db( db_ ),
    ups( ups_ )
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
		// Stored scaffold node is taken to be in main tree
		XTreeZone scaffold_tree_zone = XTreeZone::CreateFromScaffold( scaffold_base_xlink, 
		                                                              DBCommon::TreeOrdinal::MAIN ); 
		MutableTreeZone scaffold_mutable_tree_zone = db->CreateMutableTreeZone( scaffold_tree_zone.GetBaseXLink(),
													                           	scaffold_tree_zone.GetTerminusXLinks(),
													                           	DBCommon::TreeOrdinal::MAIN );		
        // TODO Down to here in a helper fn
        
		MutableTreeZone &moving_zone = p.second;
		db->SwapTreeToTree( moving_zone, vector<MutableTreeZone *>(),
							scaffold_mutable_tree_zone, vector<MutableTreeZone *>() );
		db->TeardownTree(moving_zone.GetTreeOrdinal()); // Don't leak it									                                     	
	}
}

