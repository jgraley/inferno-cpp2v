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
    node_table( make_shared<NodeTable>() ),
    orderings( make_shared<Orderings>(lacing, this) ),
    domain_extension( make_shared<DomainExtension>(this, domain_extenders) )
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
		auto tree_as_zone = XTreeZone::CreateSubtree(root_xlink);
        db_walker.WalkTreeZone( actions, tree_as_zone.get(), p.first, wind, DBCommon::GetRootCoreInfo() );
	}
}
                                 

void XTreeDatabase::MainTreeBuild(TreePtr<Node> main_root)
{      
    INDENT("=i");

    TRACE("Walk for intrinsic: orderings\n");
	auto main_free_zone = FreeZone::CreateSubtree(main_root);
	orderings->InsertIntrinsic(main_free_zone.get());   

	// Make main tree using FZ, and provide a TZ TODO pop out into eg MainTreeInstallAtrRoot(), asserting no terminii
    auto sp_main_root = make_shared<TreePtr<Node>>(main_free_zone->GetBaseNode());
    trees_by_ordinal[DBCommon::TreeOrdinal::MAIN] = {sp_main_root};
    next_tree_ordinal = DBCommon::TreeOrdinal::EXTRAS;
    XLink main_root_xlink = GetRootXLink(DBCommon::TreeOrdinal::MAIN);
    ASSERT( main_root_xlink );
	auto main_tree_zone = XTreeZone::CreateSubtree(main_root_xlink);

    TRACE("Walk for geometric: domain, tables\n");
    DBWalk::Actions actions;
    actions.push_back( bind(&Domain::InsertGeometricAction, domain.get(), placeholders::_1) ); // TODO all these into the correpsonding classes
    actions.push_back( bind(&LinkTable::InsertGeometricAction, link_table.get(), placeholders::_1) );
    actions.push_back( bind(&NodeTable::InsertGeometricAction, node_table.get(), placeholders::_1) );
	db_walker.WalkTreeZone( &actions, main_tree_zone.get(), DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_IN, DBCommon::GetRootCoreInfo() );    	

    TRACE("Walk for geometric: orderings\n");
	orderings->MainTreeInsertGeometric(main_tree_zone.get(), DBCommon::GetRootCoreInfo());   

    TRACE("Domain extension init\n");
    domain_extension->MainTreeBuild();    
    PerformDeferredDomainExcetionActions(); // More consistent if calls came via tree update layer
        
    // ---------- Relation checks ------------
    orderings->CheckRelations( link_table->GetXLinkDomainAsVector(),
                               node_table->GetNodeDomainAsVector() );
}


void XTreeDatabase::MainTreeExchange( MutableTreeZone *target_tree_zone, FreeZone *free_zone, vector<MutableTreeZone *> fixups, bool delete_intrinsics )
{
    TRACE("Replacing target TreeZone:\n")(*target_tree_zone)("\nwith source FreeZone:\n")(*free_zone)("\n");
    ASSERT( target_tree_zone->GetNumTerminii() == free_zone->GetNumTerminii() )
          ("Target TZ:%lu, source FZ:%lu", target_tree_zone->GetNumTerminii(), free_zone->GetNumTerminii());    

	target_tree_zone->Validate(this); 

    // Store the core info for the base locally since the link table will change
    // as this function executes.
    const DBCommon::CoreInfo base_info = link_table->GetCoreInfo( target_tree_zone->GetBaseXLink() );

    // Remove geometric info that will be invalidated by the exchange 
    MainTreeDeleteGeometric( target_tree_zone, &base_info, delete_intrinsics );   
    
    // Update the tree. mutable_target_tree_zone becomes the valid new tree zone.
    target_tree_zone->Exchange( free_zone, fixups ); 
    
    // Re-insert geometric info based on new tree zone
    MainTreeInsertGeometric( target_tree_zone, &base_info );       
    
#ifndef DEFERRED_AT_VERY_END
    // Update domain extension extra trees
    PerformDeferredDomainExcetionActions();
#endif
        
    if( ReadArgs::test_db )
        CheckGeometric();
}


void XTreeDatabase::MainTreeInsertGeometric(TreeZone *zone, const DBCommon::CoreInfo *base_info)
{
    INDENT("+g");

    TRACE("Walk for geometric: domain, tables\n");
    DBWalk::Actions actions;
    actions.push_back( bind(&Domain::InsertGeometricAction, domain.get(), placeholders::_1) );
    actions.push_back( bind(&LinkTable::InsertGeometricAction, link_table.get(), placeholders::_1) );
    actions.push_back( bind(&NodeTable::InsertGeometricAction, node_table.get(), placeholders::_1) );
    db_walker.WalkTreeZone( &actions, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_IN, base_info );

    TRACE("Walk for geometric: orderings\n");
    orderings->MainTreeInsertGeometric(zone, base_info);
    
    // Domain extension wants to roam around the XTree, consulting
    // parents, children, anything really. So we need a separate pass.
    TRACE("Walk for geometric: domain extension\n");
    DBWalk::Actions actions2;
    actions2.push_back( bind(&DomainExtension::InsertGeometricAction, domain_extension.get(), placeholders::_1) );
    db_walker.WalkTreeZone( &actions2, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_IN, base_info );
}


void XTreeDatabase::MainTreeDeleteGeometric(TreeZone *zone, const DBCommon::CoreInfo *base_info, bool delete_intrinsics)
{
    INDENT("-g");
    
    TRACE("Walk for geometric: domain extension\n");
    DBWalk::Actions actions;
    actions.push_back( bind(&DomainExtension::DeleteGeometricAction, domain_extension.get(), placeholders::_1) );
    db_walker.WalkTreeZone( &actions, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_OUT, base_info );   

    TRACE("Walk for geometric: orderings\n");
    orderings->MainTreeDeleteGeometric(zone, base_info, delete_intrinsics);

    TRACE("Walk for geometric: domain, tables\n");
    DBWalk::Actions actions2;
    actions2.push_back( bind(&NodeTable::DeleteGeometricAction, node_table.get(), placeholders::_1) );
    actions2.push_back( bind(&LinkTable::DeleteGeometricAction, link_table.get(), placeholders::_1) );
    actions2.push_back( bind(&Domain::DeleteGeometricAction, domain.get(), placeholders::_1) );
    db_walker.WalkTreeZone( &actions2, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_OUT, base_info );   
}


void XTreeDatabase::InsertIntrinsic(FreeZone *zone)
{
    INDENT("+i");

    TRACE("Walk for intrinsic: orderings\n");
    orderings->InsertIntrinsic(zone);
}


void XTreeDatabase::DeleteIntrinsic( FreeZone *zone )
{
    INDENT("-i");
    
    TRACE("Walk for intrinsic: orderings\n");
    orderings->DeleteIntrinsic(zone);
}


void XTreeDatabase::PerformDeferredDomainExcetionActions()
{
    domain_extension->PerformDeferredActions();
}


void XTreeDatabase::ExtraTreeBuild(DBCommon::TreeOrdinal tree_ordinal, TreePtr<Node> root_node)
{        
	INDENT("+e");
	
	TRACE("Walk for intrinsic: orderings\n");
	auto free_zone = FreeZone::CreateSubtree(root_node);
	orderings->InsertIntrinsic(free_zone.get());   
	
    auto sp_root = make_shared<TreePtr<Node>>(root_node);
    trees_by_ordinal[tree_ordinal] = {sp_root};	
    ASSERT( tree_ordinal >= DBCommon::TreeOrdinal::EXTRAS );
    XLink root_xlink = GetRootXLink(tree_ordinal);
    auto tree_as_zone = XTreeZone::CreateSubtree(root_xlink);
	TRACE("Tree ordinal: %d root: ", tree_ordinal)(tree_as_zone)("\n");
    
    TRACE("Walk for geometric: domain, tables\n");
    DBWalk::Actions actions;
    actions.push_back( bind(&Domain::InsertGeometricAction, domain.get(), placeholders::_1) );
    actions.push_back( bind(&LinkTable::InsertGeometricAction, link_table.get(), placeholders::_1) );
    actions.push_back( bind(&NodeTable::InsertGeometricAction, node_table.get(), placeholders::_1) );
    db_walker.WalkTreeZone( &actions, tree_as_zone.get(), tree_ordinal, DBWalk::WIND_IN, DBCommon::GetRootCoreInfo() );

    TRACE("Walk for geometric: orderings\n");
    orderings->MainTreeInsertGeometric(tree_as_zone.get(), DBCommon::GetRootCoreInfo());
    
    TRACE("Walk for geometric: domain extension\n");
    DBWalk::Actions actions2;
    actions2.push_back( bind(&DomainExtension::InsertGeometricAction, domain_extension.get(), placeholders::_1) );
    db_walker.WalkTreeZone( &actions2, tree_as_zone.get(), tree_ordinal, DBWalk::WIND_IN, DBCommon::GetRootCoreInfo() );
}


void XTreeDatabase::ExtraTreeTeardown(DBCommon::TreeOrdinal tree_ordinal)
{        
	INDENT("-e");
    ASSERT( tree_ordinal >= DBCommon::TreeOrdinal::EXTRAS );
    XLink root_xlink = GetRootXLink(tree_ordinal);
    auto tree_as_zone = XTreeZone::CreateSubtree(root_xlink);
	TRACE("Tree ordinal: %d root: ", tree_ordinal)(tree_as_zone)("\n");

    // Note not symmetrical with InsertExtra(): we
    // will be invoked with every xlink in the extra
    // zones and on each call we delete just that
    // xlink.
    TRACE("Walk for geometric: domain extension\n");
    DBWalk::Actions actions;
    actions.push_back( bind(&DomainExtension::DeleteGeometricAction, domain_extension.get(), placeholders::_1) );
    db_walker.WalkTreeZone( &actions, tree_as_zone.get(), tree_ordinal, DBWalk::WIND_OUT, DBCommon::GetRootCoreInfo() );       

    TRACE("Walk for geometric: orderings\n");
    orderings->MainTreeDeleteGeometric(tree_as_zone.get(), DBCommon::GetRootCoreInfo(), true);

    TRACE("Walk for geometric: domain, tables\n");
    DBWalk::Actions actions2;
    actions2.push_back( bind(&NodeTable::DeleteGeometricAction, node_table.get(), placeholders::_1) );
    actions2.push_back( bind(&LinkTable::DeleteGeometricAction, link_table.get(), placeholders::_1) );
    actions2.push_back( bind(&Domain::DeleteGeometricAction, domain.get(), placeholders::_1) );
    db_walker.WalkTreeZone( &actions2, tree_as_zone.get(), tree_ordinal, DBWalk::WIND_OUT, DBCommon::GetRootCoreInfo() );       

	auto free_zone = FreeZone::CreateSubtree(tree_as_zone->GetBaseNode());
	FreeExtraTree( tree_ordinal );
	
	// Pulled out to end because intrinsic delete are now done after all geoms
    TRACE("Walk for intrinsic: orderings\n");
	orderings->DeleteIntrinsic(free_zone.get());   
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


unique_ptr<MutableTreeZone> XTreeDatabase::CreateMutableTreeZone(XLink base,
                                                               vector<XLink> terminii)
{
	Mutator base_mutator = CreateTreeMutator(base);
	vector<Mutator> terminii_mutators;
	for( XLink t : terminii )
		terminii_mutators.push_back( CreateTreeMutator(t) ); 
	return make_unique<MutableTreeZone>( move(base_mutator), move(terminii_mutators) );
}                                                


void XTreeDatabase::Dump() const
{
    orderings->Dump();
}


void XTreeDatabase::CheckGeometric()
{
    INDENT("?");

    // ---------- Checks against a fresh "reference" build ------------
    // No deps on other parts of DB so check first
    TRACE("Making reference domain for checks\n");
    auto ref_domain = make_shared<Domain>();
    DBWalk::Actions actions1 { bind(&Domain::InsertGeometricAction, ref_domain.get(), placeholders::_1) };
    WalkAllTrees( &actions1, DBWalk::WIND_IN );
    TRACE("Checking\n");
    Domain::CheckEqual(ref_domain, domain);
    
    // Orderings have deps on LinkTable for finding parent
    TRACE("Making reference orderings for checks\n");
    auto ref_orderings = make_shared<Orderings>(lacing, this);
    
    for( auto p : trees_by_ordinal )
	{
	    XLink root_xlink = GetRootXLink(p.first);
		auto tree_as_zone = XTreeZone::CreateSubtree(root_xlink);
		ref_orderings->MainTreeInsertGeometric(tree_as_zone.get(), DBCommon::GetRootCoreInfo());
	}   
    
    TRACE("Checking\n");
    Orderings::CheckEqual(ref_orderings, orderings, false);

    // ---------- Relation checks ------------
    // Do these last as they have more deps on other DB stuff
    orderings->CheckRelations( link_table->GetXLinkDomainAsVector(),
                               node_table->GetNodeDomainAsVector() );
}


void XTreeDatabase::CheckIntrinsic()
{
    INDENT("?");

    // ---------- Checks against a fresh "reference" build ------------

    // Orderings have deps on LinkTable for finding parent
    TRACE("Making reference orderings for checks\n");
    auto ref_orderings = make_shared<Orderings>(lacing, this);
 
    for( auto p : trees_by_ordinal )
	{
	    XLink root_xlink = GetRootXLink(p.first);
		auto tree_as_zone = XTreeZone::CreateSubtree(root_xlink);
		auto free_zone = FreeZone::CreateSubtree(tree_as_zone->GetBaseNode());
		ref_orderings->InsertIntrinsic(free_zone.get());
	}   
 
    TRACE("Checking\n");
    Orderings::CheckEqual(ref_orderings, orderings, true);

    // ---------- Relation checks ------------
    // Do these last as they have more deps on other DB stuff
    orderings->CheckRelations( link_table->GetXLinkDomainAsVector(),
                               node_table->GetNodeDomainAsVector() );
}
