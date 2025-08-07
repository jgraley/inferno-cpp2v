#include "scaffold_ops.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "up_common.hpp"

using namespace SR;


ScaffoldOps::ScaffoldOps( XTreeDatabase *db_ ) :
    db( db_ )
{
}


TreeZone ScaffoldOps::FreeZoneIntoExtraTree( FreeZone free_zone, const TreeZone &tree_zone_for_plug_types )
{
	// ------------------------- Plug the zone to make a subtree ---------------------------
	// Reference tree zone is used to determine the types of the plugs
	vector<XLink> terminii;
    FreeZone::TerminusIterator terminus_it = free_zone.GetTerminiiBegin();
    for( size_t i=0; i<tree_zone_for_plug_types.GetNumTerminii(); i++ )
	{
		// Plug the terminii of the "from" scaffold with yet more scaffolding so we get a subtree for the extra tree.
		// This is a requirement for placing a zone (generally including terminii) into its own extra tree. Alternatively
		// we could allow NULL TreePtrs/placeholders to exist in tree and define semantics for them.
		TreePtr<Node> term_child_node = tree_zone_for_plug_types.GetTerminusXLink(i).GetChildTreePtr();
		ASSERT(term_child_node);
		FreeZone plug = CreateScaffoldToSpec(term_child_node, 0); // finally no terminii!!!
		XLink resulting_xlink;
        terminus_it = free_zone.MergeTerminus( terminus_it, make_unique<FreeZone>(plug), &resulting_xlink );
        terminii.push_back(resulting_xlink);
	}	
	TRACE("Free zone after populating terminii: ")(free_zone)("\n");

	// ------------------------- Build into a new extra tree ---------------------------
    // Add a new extra tree containing the plugged "from" scaffold
    DBCommon::TreeOrdinal extra_tree_ordinal = db->BuildTree( DBCommon::TreeType::UPDATE, free_zone );
	TRACE("Built extra tree %u\n", extra_tree_ordinal);    

	// ------------------------- Get unplugged zone for our scaffold ---------------------------
	// We require a TZ based on the "from" scaffold that resembles main_tree_zone_from, with real
	// TZ terminii, even though we plugged the FZ terminii making it a subtree.
	XLink root_xlink = db->GetRootXLink(extra_tree_ordinal);
	TRACE("Extra tree root: ")(root_xlink)("\n");
	TreeZone tree_zone_in_extra( root_xlink, terminii, extra_tree_ordinal );	
	TRACE("Original zone in TZ with its terminii: ")(tree_zone_in_extra)("\n");
	
	return tree_zone_in_extra;
}


TreeZone ScaffoldOps::TreeZoneAroundScaffoldNode( TreePtr<Node> scaffold_node, DBCommon::TreeOrdinal tree_ordinal )
{
	const NodeTable::Row &scaffold_row = db->GetNodeRow(scaffold_node);
	ASSERT( scaffold_row.incoming_xlinks.size() == 1 );		
	XLink base_xlink = SoloElementOf(scaffold_row.incoming_xlinks);
	
	// Stored scaffold node is taken to be in main tree
	ScaffoldBase *scaffold_ptr = dynamic_cast<ScaffoldBase *>(scaffold_node.get());

	vector<XLink> terminii;
	for( TreePtr<Node> &terminus_tpi : scaffold_ptr->child_ptrs )
		terminii.push_back( XLink( &terminus_tpi ) );
	
	return TreeZone( base_xlink, terminii, tree_ordinal );
}


FreeZone ScaffoldOps::CreateSimilarScaffoldZone(const Zone &zone) const
{
	return CreateScaffoldToSpec(zone.GetBaseNode(), zone.GetNumTerminii());
}


FreeZone ScaffoldOps::CreateScaffoldToSpec(TreePtr<Node> base, int num_terminii) const
{
	//auto base = (TreePtr<Node>)(*tpi_base);
	ASSERTS(base);
	const TreeUtilsInterface *upi = base->MakeTP();
	ASSERTS(upi);
	TreePtr<Node> scaffold = upi->MakeScaffold();

	ScaffoldBase *sbp = dynamic_cast<ScaffoldBase *>(scaffold.get());
	ASSERTS( sbp );
	Sequence<Node> *ssp = &(sbp->child_ptrs);
 
    // Set the base as the scaffolding node
    auto zone = FreeZone( scaffold, list<Mutator>{} );
    
    // Set the terminii as the scaffolding node's scaffold child pointers (the
    // underlying node type's children will be left empty/NULL)
    for( int i=0; i<num_terminii; i++ )
    {
        ContainerInterface::iterator it = ssp->insert( Mutator::MakePlaceholder() );
        zone.AddTerminus( Mutator::CreateFreeContainer(scaffold, ssp, it) );     
    }
    
    //FTRACES("Created scaffold with %d terminii\n", num_terminii)("\n");

    return zone;
}



