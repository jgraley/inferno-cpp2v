#include "x_tree_database.hpp"

#include "lacing.hpp"

#include "common/read_args.hpp"
#include "tree_zone.hpp"
#include "free_zone.hpp"
#include "mutable_zone.hpp"

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
    next_tree_ordinal( 0 )
{
}

    
DBCommon::TreeOrdinal XTreeDatabase::BuildTree( DBCommon::TreeType tree_type, const FreeZone &free_zone)
{      
    INDENT("B");
    DBCommon::TreeOrdinal tree_ordinal = AllocateTree();
    
    TRACE("Walk for intrinsic: orderings\n");
    auto sp_root = make_shared<TreePtr<Node>>(free_zone.GetBaseNode());
    trees_by_ordinal[tree_ordinal] = {sp_root, sp_root.get(), tree_type};
    XLink root_xlink = GetRootXLink(tree_ordinal);
    ASSERT( root_xlink );
	auto zone = TreeZone::CreateSubtree(root_xlink, tree_ordinal);
    TRACE("Tree ordinal: %d subtree zone: ", tree_ordinal)(zone)("\n");

	domain->InsertTree(zone);   
	link_table->InsertTree(zone);   
	node_table->InsertTree(zone);   
	orderings->InsertTree(zone);   
    domain_extension->InsertTree(zone);   
    
    if( tree_type == DBCommon::TreeType::MAIN )
		main_tree_ordinal = tree_ordinal;
    
    CheckAssets();       
    
    return tree_ordinal;
}


void XTreeDatabase::TeardownTree(DBCommon::TreeOrdinal tree_ordinal)
{        
	INDENT("T");
    {
		XLink root_xlink = GetRootXLink(tree_ordinal);
		auto zone = TreeZone::CreateSubtree(root_xlink, tree_ordinal);
		TRACE("Tree ordinal: %d root: ", tree_ordinal)(zone)("\n");

		domain_extension->DeleteTree(zone);   
		orderings->DeleteTree(zone);
		node_table->DeleteTree(zone);   
		link_table->DeleteTree(zone);   
		domain->DeleteTree(zone);   
		
		// XLink memory safety: let zone and root_xlink drop out of scope 
		// before freeing tree, which will delete the underlying TreePtr<>	
	} 

	deferred_tree_ordinals.push( tree_ordinal );
    
    CheckAssets();       	
}


void XTreeDatabase::SwapTreeToTree( TreeZone &zone1, vector<TreeZone *> fixups1,
                                    TreeZone &zone2, vector<TreeZone *> fixups2 )
{	
	INDENT("S");
    TRACE("Swapping target TreeZones:\n")(zone1)
         ("\nand: ")(zone2);
    ASSERT( zone1.GetNumTerminii() == zone2.GetNumTerminii() )
          ("left TZ:%lu, right TZ:%lu", zone1.GetNumTerminii(), zone2.GetNumTerminii());    
	// TZs must be in different trees to avoid interference. This could result from a 
	// shared boundary, or more remote action like the SC ordering deleting all ancestors
	// of the base of a zone.
	ASSERT( zone1.GetTreeOrdinal() != zone2.GetTreeOrdinal() );

	//zone1.Validate(this); 
	//zone2.Validate(this); 
    
	// Move to local mutable zone here because a suspended LinkTable will not 
	// be able to provide the info to create them.
	MutableZone lmzone1 = CreateMutableZone(zone1);
	MutableZone lmzone2 = CreateMutableZone(zone2);

	{
		// Scope contains suspension objects on stack
		DomainExtension::SwapTransaction de_sus(domain_extension.get(), zone1, zone2);  
		Orderings::SwapTransaction orderings_sus(orderings.get(), zone1, zone2);  
		NodeTable::SwapTransaction node_table_sus(node_table.get(), zone1, zone2);  
		LinkTable::SwapTransaction link_table_sus(link_table.get(), zone1, zone2);  
		
		MutableZone::Swap( lmzone1, fixups1, 
		                   lmzone2, fixups2 );  

		// Fix up the supplied zones
		zone1 = lmzone1.GetXTreeZone();
		zone2 = lmzone2.GetXTreeZone();		        

		TRACE("After swapping zones: ")(lmzone1)
			 ("\nand: ")(lmzone2)("\n");    
			 
		// Suspensions expire in reverse order.     
	} 
        
    CheckAssets();       
}


void XTreeDatabase::DeferredActionsEndOfUpdate()
{
    domain_extension->DeferredActionsEndOfUpdate();
    CheckAssets();           
}


void XTreeDatabase::DeferredActionsEndOfStep()
{
	// This part would be better done by returning the deferred ordinals
	// to the SCR engine and letting it hold them in local scope, so that
	// the right trees are deleted at the right time (i.e. on the unwind
	// of its recursion into embeddeds) TODO #823
	while( !deferred_tree_ordinals.empty() )
	{		
		FreeTree( deferred_tree_ordinals.front() );  
		deferred_tree_ordinals.pop();
	}
	
	domain_extension->DeferredActionsEndOfStep();
    CheckAssets();           
}


XLink XTreeDatabase::GetRootXLink(DBCommon::TreeOrdinal tree_ordinal) const
{
    const DBCommon::TreeRecord &tree_rec = trees_by_ordinal.at(tree_ordinal);
    return XLink::CreateFrom( tree_rec.tpi_root_node );
}


DBCommon::TreeType XTreeDatabase::GetTreeType(DBCommon::TreeOrdinal tree_ordinal) const
{
	return trees_by_ordinal.at(tree_ordinal).type;
}


const DomainExtensionChannel *XTreeDatabase::GetDEChannel( const DomainExtension::Extender *extender ) const
{
    return domain_extension->GetChannel(extender);
}


const Domain &XTreeDatabase::GetDomain() const
{
    return *domain;
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
    return GetRootXLink(main_tree_ordinal);
}


DBCommon::TreeOrdinal XTreeDatabase::GetMainTreeOrdinal() const
{
    return main_tree_ordinal;
}


DBCommon::TreeOrdinal XTreeDatabase::GetTreeOrdinalFor(XLink xlink) const
{
	while(XLink next_xlink = TryGetParentXLink(xlink)) 
		xlink = next_xlink;
	
	LinkTable::Row row = link_table->GetRow(xlink);
	ASSERT( row.context_type == DBCommon::Context::ROOT );
	return row.tree_ordinal;		
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

	size_t tot_num_xlinks = domain->GetTotNumXLinks();
	size_t tot_num_nodes = node_table->GetTotNumNodes();

	link_table->CheckSizeIs(tot_num_xlinks);
	orderings->CheckSizeIs(tot_num_xlinks, tot_num_nodes);

	// ---------- Check node table incomings -------------		
	node_table->ConsistencyCheck(this);

	if( !ReadArgs::test_db )
		return;
		
    // ---------- Checks against a fresh "reference" build ------------
    // No deps on other parts of DB so check first
    TRACE("Making reference domain for checks\n");
    auto ref_domain = make_shared<Domain>();
    for( auto p : trees_by_ordinal )
	{
	    XLink root_xlink = GetRootXLink(p.first);
		auto tree_zone = TreeZone::CreateSubtree(root_xlink);
		ref_domain->InsertTree(tree_zone);
	}   

    TRACE("Checking domain\n");
    Domain::CheckEqual(ref_domain, domain);
    
    // Orderings have deps on LinkTable for finding parent
    TRACE("Making reference orderings for checks\n");
    auto ref_orderings = make_shared<Orderings>(lacing, this);
    
    for( auto p : trees_by_ordinal )
	{
	    XLink root_xlink = GetRootXLink(p.first);
		auto tree_zone = TreeZone::CreateSubtree(root_xlink);
		ref_orderings->InsertTree(tree_zone);
	}   
    
    TRACE("Checking orderings\n");
    Orderings::CheckEqual(ref_orderings, orderings, false);

    // ---------- Relation checks ------------
    // Do these last as they have more deps on other DB stuff
    orderings->CheckRelations( link_table->GetXLinkDomainAsVector(),
                               node_table->GetNodeDomainAsVector() );
}


Mutator XTreeDatabase::CreateTreeMutator(XLink xlink)  const
{
    const LinkTable::Row &row = link_table->GetRow(xlink);   
    Mutator m;
    
    switch( row.context_type )
    {
        case DBCommon::ROOT:
        {
            // This shared_ptr points to the same TreePtr<> instance referenced by the root XLink.
            // By holding it separately, we can avoid a const cast, and in fact constness propagates
            // correctly from the XTreeDatabase object, which is why this method cannot be const.
            ASSERT( (int)(row.tree_ordinal) >= 0 ); // Should be valid whenever context is ROOT
            const DBCommon::TreeRecord &tree_rec = trees_by_ordinal.at(row.tree_ordinal);
            m = Mutator::CreateTreeRoot( tree_rec.sp_tp_root_node, tree_rec.tpi_root_node );
            break;
        }    
        case DBCommon::SINGULAR:
        {
            const vector< Itemiser::Element * > x_items = row.parent_node->Itemise();
            Itemiser::Element *xe = x_items.at(row.item_ordinal);        
            auto p_x_singular = dynamic_cast<TreePtrInterface *>(xe);
            ASSERT( p_x_singular );
            m = Mutator::CreateTreeSingular( row.parent_node, p_x_singular );
            break;
        }
        case DBCommon::IN_SEQUENCE:
        case DBCommon::IN_COLLECTION: 
        {
            // COLLECTION is the motivating case: its elements are const, so we neet Mutate() to change them
            m = Mutator::CreateTreeContainer( row.parent_node, row.p_container, row.container_it );  
            break;          
        }
        default:
        {
            ASSERTFAIL(); // Base of free zone is just a node, so there's no unique mutator for it
        }
    }    
#ifdef KEEP_WHODAT_INFO
	m.SetWhodat( xlink.GetWhodat() );
#endif
	return m;
}


MutableZone XTreeDatabase::CreateMutableZone(TreeZone &zone) const
{
	Mutator base_mutator = CreateTreeMutator(zone.GetBaseXLink());
	vector<Mutator> terminii_mutators;
	for( XLink t : zone.GetTerminusXLinks() )
		terminii_mutators.push_back( CreateTreeMutator(t) ); 
	return MutableZone( move(base_mutator), move(terminii_mutators), zone.GetTreeOrdinal() );
}                                                


DBCommon::TreeOrdinal XTreeDatabase::AllocateTree()
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


void XTreeDatabase::FreeTree(DBCommon::TreeOrdinal tree_ordinal)
{
    trees_by_ordinal.erase(tree_ordinal);
    free_tree_ordinals.push(tree_ordinal);
}
