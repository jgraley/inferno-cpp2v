#include "update_ops.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"

using namespace SR;


UpdateOps::UpdateOps( XTreeDatabase *db_ ) :
    db( db_ )
{
}


MutableTreeZone UpdateOps::FreeZoneIntoExtraTree( FreeZone free_zone, const MutableTreeZone &reference_tree_zone )
{
	// Reference tree zone is used to determine the types of the plugs

	vector<XLink> terminii;
    FreeZone::TerminusIterator terminus_it = free_zone.GetTerminiiBegin();
    for( size_t i=0; i<reference_tree_zone.GetNumTerminii(); i++ )
	{
		// Plug the terminii of the "from" scaffold with yet more scaffolding so we get a subtree for the extra tree.
		// This is a requirement for placing a zone (generally including terminii) into its own extra tree. Alternatively
		// we could allow NULL TreePtrs/placeholders to exist in tree and define semantics for them.
		TreePtr<Node> term_child_node = reference_tree_zone.GetTerminusMutator(i).GetChildTreePtr();
		ASSERT(term_child_node);
		FreeZone plug = FreeZone::CreateScaffoldToSpec(term_child_node, 0); // finally no terminii!!!
		Mutator resultant_mutator;
        terminus_it = free_zone.MergeTerminus( terminus_it, make_unique<FreeZone>(plug), &resultant_mutator );
        terminii.push_back(resultant_mutator.GetXLink());
	}	
	TRACE("Free zone after populating terminii: ")(free_zone)("\n");

    // Add a new extra tree containing the plugged "from" scaffold
    DBCommon::TreeOrdinal extra_tree_ordinal = db->AllocateExtraTree();        
	TRACE("Allocated extra tree %u\n", extra_tree_ordinal);
    MutableTreeZone zone_in_extra_tree = db->BuildTree( extra_tree_ordinal, free_zone );
	TRACE("Zone in extra tree: ")(zone_in_extra_tree)("\n");

	// ------------------------- Get unplugged zone for our scaffold ---------------------------
	// We require a TZ based on the "from" scaffold that resembles main_tree_zone_from, with real
	// TZ terminii, even though we plugged the FZ terminii making it a subtree.
	XLink root_xlink = db->GetRootXLink(extra_tree_ordinal);
	TRACE("Extra tree root: ")(root_xlink)("\n");
	MutableTreeZone tree_zone_in_extra = db->CreateMutableTreeZone( root_xlink, terminii, extra_tree_ordinal );	
	TRACE("Original zone in TZ with its terminii: ")(tree_zone_in_extra)("\n");
	
	return tree_zone_in_extra;
}



