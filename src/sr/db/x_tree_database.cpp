#include "x_tree_database.hpp"

#include "lacing.hpp"

#include "common/read_args.hpp"
#include "tree_zone.hpp"
#include "free_zone.hpp"

using namespace SR;    

// We won't normally expect matches as postconditions to our
// public methods because changing strategies make some do more
// and others do less. But:
// - FullX methods should erase history, so can always check
// - All methods should match if strategy does not change, as
//   when testing the test
//#define DB_TEST_THE_TEST

XTreeDatabase::XTreeDatabase( shared_ptr<Lacing> lacing_, DomainExtension::ExtenderSet domain_extenders ) :
    lacing( lacing_ ),
    domain( make_shared<Domain>() ),
    link_table( make_shared<LinkTable>() ),
    node_table( make_shared<NodeTable>(link_table.get()) ),
    orderings( make_shared<Orderings>(lacing, this) ),
    domain_extension( make_shared<DomainExtension>(this, domain_extenders) ),
    next_tree_ordinal( DBCommon::TreeOrdinal::EXTRAS ) // because main is always allocated
{
}

    
DBCommon::TreeOrdinal XTreeDatabase::AllocateExtraTree()
{
    DBCommon::TreeOrdinal assigned_ordinal;
    if( free_tree_ordinals.empty() )
    {
        assigned_ordinal = next_tree_ordinal;
        ((int &)next_tree_ordinal)++;
    }
    else
    {
        assigned_ordinal = free_tree_ordinals.front();
        free_tree_ordinals.pop();
    }
	
    return assigned_ordinal;
}

    
void XTreeDatabase::FreeExtraTree(DBCommon::TreeOrdinal tree_ordinal)
{
    trees_by_ordinal.erase(tree_ordinal);
    free_tree_ordinals.push(tree_ordinal);
}


XLink XTreeDatabase::GetRootXLink(DBCommon::TreeOrdinal tree_ordinal) const
{
    return XLink::CreateFrom( trees_by_ordinal.at(tree_ordinal).sp_tp_root_node );
}


vector<XLink> XTreeDatabase::GetExtraRootXLinks() const
{
    vector<XLink> xlinks;
    for( auto p : trees_by_ordinal )
    {
        if( p.first != DBCommon::TreeOrdinal::MAIN )
        {
            xlinks.push_back( XLink::CreateFrom( p.second.sp_tp_root_node ) );
        }
    }
    
    return xlinks;
}


void XTreeDatabase::WalkAllTrees(const DBWalk::Actions *actions,
                                 DBWalk::Wind wind)
{
    for( auto p : trees_by_ordinal )
    {
	    XLink root_xlink = GetRootXLink(p.first);
		auto tree_zone = XTreeZone::CreateSubtree(root_xlink);
        db_walker.WalkTreeZone( actions, tree_zone, p.first, wind, DBCommon::GetRootCoreInfo() );
	}
}
                                 

MutableTreeZone XTreeDatabase::BuildTree(DBCommon::TreeOrdinal tree_ordinal, const FreeZone &free_zone)
{      
    INDENT("+t");
    ASSERT( tree_ordinal >= DBCommon::TreeOrdinal(0) );
    
    TRACE("Walk for intrinsic: orderings\n");
    auto sp_root = make_shared<TreePtr<Node>>(free_zone.GetBaseNode());
    trees_by_ordinal[tree_ordinal] = {sp_root};
    XLink root_xlink = GetRootXLink(tree_ordinal);
    ASSERT( root_xlink );
	auto zone = XTreeZone::CreateSubtree(root_xlink);
    TRACE("Tree ordinal: %d subtree zone: ", tree_ordinal)(zone)("\n");

	const DBCommon::CoreInfo *base_info = DBCommon::GetRootCoreInfo();

	domain->Insert(zone, base_info, true);   
	link_table->Insert(tree_ordinal, zone, base_info, true);   
	node_table->Insert(zone, base_info, true);   
	orderings->Insert(zone, base_info, true);   
    domain_extension->Insert(zone, base_info, true);   
    
	vector<Mutator> terminii;
	for( FreeZone::TerminusConstIterator it=free_zone.GetTerminiiBegin(); 
		 it != free_zone.GetTerminiiEnd(); 
		 it++ )
		terminii.push_back( *it ); 
	MutableTreeZone tree_zone(CreateTreeMutator(root_xlink), move(terminii));
    return tree_zone;
}


void XTreeDatabase::TeardownTree(DBCommon::TreeOrdinal tree_ordinal)
{        
	INDENT("-t");
    ASSERT( tree_ordinal >= DBCommon::TreeOrdinal::EXTRAS );
    XLink root_xlink = GetRootXLink(tree_ordinal);
    auto zone = XTreeZone::CreateSubtree(root_xlink);
	TRACE("Tree ordinal: %d root: ", tree_ordinal)(zone)("\n");

	const DBCommon::CoreInfo *base_info = DBCommon::GetRootCoreInfo();

    domain_extension->Delete(zone, base_info, true);   
    orderings->Delete(zone, base_info, true);
	node_table->Delete(zone, base_info, true);   
	link_table->Delete(tree_ordinal, zone, base_info, true);   
	domain->Delete(zone, base_info, true);   

	FreeExtraTree( tree_ordinal );  
}


void XTreeDatabase::SwapTreeToTree( DBCommon::TreeOrdinal tree_ordinal_l, MutableTreeZone &tree_zone_l, vector<MutableTreeZone *> fixups_l,
                                    DBCommon::TreeOrdinal tree_ordinal_r, MutableTreeZone &tree_zone_r, vector<MutableTreeZone *> fixups_r )
{
    TRACE("Swapping target TreeZones:\n")(tree_zone_l)(" in #%u ", tree_ordinal_l)
         ("\nand: ")(tree_zone_r)(" in #%u\n", tree_ordinal_r);
    ASSERT( tree_zone_l.GetNumTerminii() == tree_zone_r.GetNumTerminii() )
          ("left TZ:%lu, right TZ:%lu", tree_zone_l.GetNumTerminii(), tree_zone_r.GetNumTerminii());    
	// TZs must be in different trees to avoid interference. This could result from a 
	// shared boundary, or more remote action like the SC ordering deleting all ancestors
	// of the base of a zone.
	ASSERT( tree_ordinal_l != tree_ordinal_r );

	tree_zone_l.Validate(this); 
	tree_zone_r.Validate(this); 

    // Store the core info for the base locally since the link table will change
    // as this function executes.
    const DBCommon::CoreInfo base_info_l = link_table->GetCoreInfo( tree_zone_l.GetBaseXLink() );
    const DBCommon::CoreInfo base_info_r = link_table->GetCoreInfo( tree_zone_r.GetBaseXLink() );

    // Remove geometric info that will be invalidated by the exchange 
    AssetsDeleteDeux( tree_ordinal_l, tree_zone_l, &base_info_l, tree_ordinal_r, tree_zone_r, &base_info_r );   
    
    // Update the tree. mutable_target_tree_zone becomes the valid new tree zone.
    // Invariant wrt intrinisc asset state.
    tree_zone_l.Swap( tree_zone_r, fixups_l, fixups_r ); 
    
    //DumpTables();
    
    TRACE("After swapping target TreeZones:\n")(tree_zone_l)
         ("\nand: ")(tree_zone_r)("\n");    
         
    // Re-insert geometric info based on new tree zone. 
    AssetsInsertDeux( tree_ordinal_l, tree_zone_l, &base_info_l, tree_ordinal_r, tree_zone_r, &base_info_r );       
        
    if( ReadArgs::test_db )
        CheckAssets();
}

    
void XTreeDatabase::AssetsInsertDeux(DBCommon::TreeOrdinal tree_ordinal1, TreeZone &zone1, const DBCommon::CoreInfo *base_info1,
									 DBCommon::TreeOrdinal tree_ordinal2, TreeZone &zone2, const DBCommon::CoreInfo *base_info2 )
{
    INDENT("+d");

	// Geom only!!!
	domain->Insert(zone1, base_info1, false);   
	domain->Insert(zone2, base_info2, false);   
	link_table->Insert(tree_ordinal1, zone1, base_info1, false);   
	link_table->Insert(tree_ordinal2, zone2, base_info2, false);   
	node_table->Insert(zone1, base_info1, false);   
	node_table->Insert(zone2, base_info2, false);   
    orderings->Insert(zone1, base_info1, false); // doesn't use tree_ordinal
    orderings->Insert(zone2, base_info2, false); // doesn't use tree_ordinal
    domain_extension->Insert(zone1, base_info1, false);   
    domain_extension->Insert(zone2, base_info2, false); 
}


void XTreeDatabase::AssetsDeleteDeux(DBCommon::TreeOrdinal tree_ordinal1, TreeZone &zone1, const DBCommon::CoreInfo *base_info1,
									 DBCommon::TreeOrdinal tree_ordinal2, TreeZone &zone2, const DBCommon::CoreInfo *base_info2 )
{
    INDENT("-d");
    
	// Geom only!!!
    domain_extension->Delete(zone1, base_info1, false);   
    domain_extension->Delete(zone2, base_info2, false);     
    orderings->Delete(zone1, base_info1, false); // doesn't use tree_ordinal
    orderings->Delete(zone2, base_info2, false); // doesn't use tree_ordinal
	node_table->Delete(zone1, base_info1, false);   
	node_table->Delete(zone2, base_info2, false);   
	link_table->Delete(tree_ordinal1, zone1, base_info1, false);   
	link_table->Delete(tree_ordinal2, zone2, base_info2, false);   
	domain->Delete(zone1, base_info1, false);   
	domain->Delete(zone2, base_info2, false);   
}


void XTreeDatabase::PerformDeferredActions()
{
    domain_extension->PerformDeferredActions();
}


const DomainExtensionChannel *XTreeDatabase::GetDEChannel( const DomainExtension::Extender *extender ) const
{
    return domain_extension->GetChannel(extender);
}


const Domain &XTreeDatabase::GetDomain() const
{
    return *domain;
}


const LinkTable &XTreeDatabase::GetLinkTable() const
{
    return *link_table;
}


const LinkTable::Row &XTreeDatabase::GetRow(XLink xlink) const
{
    return link_table->GetRow(xlink);
}


bool XTreeDatabase::HasRow(XLink xlink) const
{
    return link_table->HasRow(xlink);
}


const NodeTable::Row &XTreeDatabase::GetNodeRow(TreePtr<Node> node) const
{
    return node_table->GetRow(node);
}


bool XTreeDatabase::HasNodeRow(TreePtr<Node> node) const
{
    return node_table->HasRow(node);
}


XLink XTreeDatabase::TryGetParentXLink(XLink xlink) const
{
    TreePtr<Node> parent_node = GetRow(xlink).parent_node;
    if( !parent_node )
        return XLink();
        
    const set<XLink> &ps = GetNodeRow(parent_node).incoming_xlinks;

    // Note that the parent is unique because:
    // - row is relative to a link, not a node,
    // - multiple parents only allowed at leaf (see #217), and parent is 
    //   (at least) one level back from that.
    switch( ps.size() )
    {
        case 0: return XLink(); 
        case 1: return SoloElementOf(ps);
        default: ASSERTFAIL("Rule #217 violation: node with child TreePtr has multiple parents");
    }        
}


XLink XTreeDatabase::GetXLink( const TreePtrInterface *px ) const
{
    XLink xlink = TryGetXLink(px);
    ASSERT( xlink )("GetXLink() could not find link for ")(px);
    return xlink;
}


XLink XTreeDatabase::TryGetXLink( const TreePtrInterface *px ) const
{
    TreePtr<Node> child_node = (TreePtr<Node>)*px;

    if( !HasNodeRow(child_node) )
        return XLink(); // fail
    
    NodeTable::Row row = GetNodeRow(child_node);

    for( XLink xlink : row.incoming_xlinks )
        if( xlink.GetTreePtrInterface() == px )
            return xlink;
    
    return XLink(); // fail
}


XLink XTreeDatabase::GetLastDescendantXLink(XLink base) 
{
    TreePtr<Node> x = base.GetChildTreePtr();
    ASSERTS(x)("This probably means we're walking an incomplete tree");
    vector< Itemiser::Element * > x_items = x->Itemise();

    // Loop backward over the items
    for( int item_ordinal=x_items.size()-1; item_ordinal>=0; item_ordinal-- )
    {
        Itemiser::Element *xe = x_items[item_ordinal];
        if( auto x_con = dynamic_cast<ContainerInterface *>(xe) )
        {
            if( !x_con->empty() )
                return GetLastDescendantXLink( XLink( x, &x_con->back() ) );
        }
        else if( auto p_x_singular = dynamic_cast<TreePtrInterface *>(xe) )
        {
            if( *p_x_singular ) // tolerate NULL singlar child pointers
                return GetLastDescendantXLink( XLink( x, &*p_x_singular ) );
        }
        else
            ASSERTFAILS("got something strange from itemise");
    }

    // No children so we are our our own last descendant
    return base;    
}


const Orderings &XTreeDatabase::GetOrderings() const
{
    return *orderings;
}


TreePtr<Node> XTreeDatabase::GetMainRootNode() const
{
    return GetMainRootXLink().GetChildTreePtr();
}


XLink XTreeDatabase::GetMainRootXLink() const
{
    return GetRootXLink(DBCommon::TreeOrdinal::MAIN);
}


Mutator XTreeDatabase::CreateTreeMutator(XLink xlink) 
{
    const LinkTable::Row &row = link_table->GetRow(xlink);
    
    switch( row.context_type )
    {
        case DBCommon::ROOT:
        {
            // This shared_ptr points to the same TreePtr<> instance referenced by the root XLink.
            // By holding it separately, we can avoid a const cast, and in fact constness propagates
            // correctly from the XTreeDatabase object, which is why this method cannot be const.
            ASSERT( (int)(row.tree_ordinal) >= 0 ); // Should be valid whenever context is ROOT
            shared_ptr<TreePtr<Node>> sp_tp_root_node = trees_by_ordinal.at(row.tree_ordinal).sp_tp_root_node;
            return Mutator::CreateTreeRoot( sp_tp_root_node );
            break;
        }    
        case DBCommon::SINGULAR:
        {
            const vector< Itemiser::Element * > x_items = row.parent_node->Itemise();
            Itemiser::Element *xe = x_items.at(row.item_ordinal);        
            auto p_x_singular = dynamic_cast<TreePtrInterface *>(xe);
            ASSERT( p_x_singular );
            return Mutator::CreateTreeSingular( row.parent_node, p_x_singular );
            break;
        }
        case DBCommon::IN_SEQUENCE:
        case DBCommon::IN_COLLECTION: 
        {
            // COLLECTION is the motivating case: its elements are const, so we neet Mutate() to change them
            return Mutator::CreateTreeContainer( row.parent_node, row.p_container, row.container_it );  
            break;          
        }
        case DBCommon::FREE_BASE:
        {
            ASSERTFAIL(); // Base of free zone is just a node, so there's no unique mutator for it
        }
        default:
        {
            ASSERTFAIL(); // Base of free zone is just a node, so there's no unique mutator for it
        }
    }    
}


MutableTreeZone XTreeDatabase::CreateMutableTreeZone(XLink base,
                                                     vector<XLink> terminii)
{
	Mutator base_mutator = CreateTreeMutator(base);
	vector<Mutator> terminii_mutators;
	for( XLink t : terminii )
		terminii_mutators.push_back( CreateTreeMutator(t) ); 
	return MutableTreeZone( move(base_mutator), move(terminii_mutators) );
}                                                


void XTreeDatabase::Dump() const
{
    orderings->Dump();
}


void XTreeDatabase::DumpTables() const
{
    link_table->Dump();
    node_table->Dump();
}


void XTreeDatabase::CheckAssets()
{
    INDENT("?");

    // ---------- Checks against a fresh "reference" build ------------
    // No deps on other parts of DB so check first
    TRACE("Making reference domain for checks\n");
    auto ref_domain = make_shared<Domain>();
    for( auto p : trees_by_ordinal )
	{
	    XLink root_xlink = GetRootXLink(p.first);
		auto tree_zone = XTreeZone::CreateSubtree(root_xlink);
		ref_domain->Insert(tree_zone, DBCommon::GetRootCoreInfo(), true);
	}   

    TRACE("Checking domain\n");
    Domain::CheckEqual(ref_domain, domain);
    
    // Orderings have deps on LinkTable for finding parent
    TRACE("Making reference orderings for checks\n");
    auto ref_orderings = make_shared<Orderings>(lacing, this);
    
    for( auto p : trees_by_ordinal )
	{
	    XLink root_xlink = GetRootXLink(p.first);
		auto tree_zone = XTreeZone::CreateSubtree(root_xlink);
		ref_orderings->Insert(tree_zone, DBCommon::GetRootCoreInfo(), true);
	}   
    
    TRACE("Checking orderings\n");
    Orderings::CheckEqual(ref_orderings, orderings, false);

    // ---------- Relation checks ------------
    // Do these last as they have more deps on other DB stuff
    orderings->CheckRelations( link_table->GetXLinkDomainAsVector(),
                               node_table->GetNodeDomainAsVector() );
}
