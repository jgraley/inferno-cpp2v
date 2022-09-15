#include "x_tree_database.hpp"
#include "sc_relation.hpp"
#include "agents/agent.hpp"
#include "vn_step.hpp"
#include "sym/expression.hpp"
#include "sym/predicate_operators.hpp"
#include "lacing.hpp"

using namespace SR;    

//#define TEST_RELATION_PROPERTIES_USING_DOMAIN
//#define TRACE_X_TREE_DB_DELTAS

#ifdef TRACE_X_TREE_DB_DELTAS
// Global because there are different x_tree_dbs owned by different SCR Engines
XTreeDatabase::CategoryOrderedIndex previous_category_ordered_domain;
#endif    


XTreeDatabase::XTreeDatabase( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    plan( clauses ),
    category_ordered_index( plan.lacing )
{ 
}


XTreeDatabase::Plan::Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    lacing( make_shared<Lacing>() )
{
    // Warning: there are a few places that declare an empty x_tree_db
    if( clauses.empty() )
        return;
    
    // Extract all the non-final archetypes from the IsInCategoryOperator nodes 
    // into a set so that they are uniqued by SimpleCompare equality. These
    // are the categories.
    Lacing::CategorySet categories;
    for( shared_ptr<SYM::BooleanExpression> clause : clauses )
    {
        clause->ForDepthFirstWalk( [&](const SYM::Expression *expr)
        {
            if( auto ko_expr = dynamic_cast<const SYM::IsInCategoryOperator *>(expr) )
            { 
                categories.insert( ko_expr->GetArchetypeNode() );
            }
        } );
    }

    lacing->Build(categories);
}

    
XLink XTreeDatabase::UniquifyDomainExtension( XLink xlink )
{
    ASSERT( xlink );
  
    // Don't worry about generated nodes that are already in 
    // the X tree (they had to have been found there after a
    // search). 
    if( unordered_domain.count(xlink) > 0 )
        return xlink;
        
    return domain_extension_classes->Uniquify( xlink ); 
}


XLink XTreeDatabase::FindDomainExtension( XLink xlink ) const
{
    ASSERT( xlink );
    
    // Don't worry about generated nodes that are already in 
    // the X tree (they had to have been found there after a
    // search). 
    if( unordered_domain.count(xlink) > 0 )
        return xlink;
        
    return domain_extension_classes->Find( xlink ); 
}


const Lacing *XTreeDatabase::GetLacing() const
{
    return plan.lacing.get();
}


XTreeDatabase::CategoryRelation::CategoryRelation()
{
}


XTreeDatabase::CategoryRelation::CategoryRelation( shared_ptr<Lacing> lacing_ ) :
    lacing( lacing_ )
{
}


XTreeDatabase::CategoryRelation& XTreeDatabase::CategoryRelation::operator=(const CategoryRelation &other)
{
    lacing = other.lacing;
    return *this;
}


bool XTreeDatabase::CategoryRelation::operator() (const XLink& x_link, const XLink& y_link) const
{
    TreePtr<Node> x = x_link.GetChildX();
    auto cat_x = TreePtr<CategoryMinimaxNode>::DynamicCast( x );
    TreePtr<Node> y = y_link.GetChildX();    
    auto cat_y = TreePtr<CategoryMinimaxNode>::DynamicCast( y );

    if( !cat_x && !cat_y )
        return lacing->IsOrdinalLess( x, y );    
   
    int xi, yi;
    if( cat_x )
        xi = cat_x->GetLacingOrdinal();
    else
        xi = lacing->GetOrdinalForNode( x );
    if( cat_y )
        yi = cat_y->GetLacingOrdinal();
    else
        yi = lacing->GetOrdinalForNode( y );
    return xi < yi;   
}


XTreeDatabase::CategoryMinimaxNode::CategoryMinimaxNode( int lacing_ordinal_ ) :
    lacing_ordinal( lacing_ordinal_ )
{
}
    

XTreeDatabase::CategoryMinimaxNode::CategoryMinimaxNode() :
    lacing_ordinal( 0 )
{
}
    

int XTreeDatabase::CategoryMinimaxNode::GetLacingOrdinal() const
{
    return lacing_ordinal;
}
 

string XTreeDatabase::CategoryMinimaxNode::GetTrace() const
{
    return GetTypeName() + SSPrintf("(%d)", lacing_ordinal);
}


const XTreeDatabase::Row &XTreeDatabase::GetRow(XLink xlink) const
{
    ASSERT( xlink );
    ASSERT( HasRow(xlink) )
          ("X tree database: no row for ")(xlink)("\n")
          ("Rows: ")(xlink_table);
    return xlink_table.at(xlink);
}


bool XTreeDatabase::HasRow(XLink xlink) const
{
    ASSERT( xlink );
    return xlink_table.count(xlink) > 0;
}


const XTreeDatabase::NodeRow &XTreeDatabase::GetNodeRow(TreePtr<Node> node) const
{
    ASSERT( node );
    ASSERT( HasNodeRow(node) )
          ("X tree database: no node row for ")(node)("\n")
          ("Node xlink_table: ")(node_table);
    return node_table.at(node);
}


bool XTreeDatabase::HasNodeRow(TreePtr<Node> node) const
{
    ASSERT( node );
    return node_table.count(node) > 0;
}


void XTreeDatabase::Build( XLink root_xlink )
{      
	ASSERT( root_xlink );
	
    // Clear everything 
    unordered_domain.clear();
    depth_first_ordered_index.clear();
    category_ordered_index.clear();
    simple_compare_ordered_index.clear();
    domain_extension_classes = make_shared<SimpleCompareQuotientSet>();
    xlink_table.clear();
    node_table.clear();    
    current_ordinal = 0;
    
    AddAtRoot( REQUIRE_SOLO, root_xlink );
    
    AddAtRoot( REQUIRE_SOLO, XLink::MMAX_Link );
    AddAtRoot( REQUIRE_SOLO, XLink::OffEndXLink );
        
#ifdef TRACE_X_TREE_DB_DELTAS
    if( Tracer::IsEnabled() ) 
    {        
        TRACE("X tree db rebuilt at ")(root_xlink)(":\n")
             ( category_ordered_index )("\n");
        previous_category_ordered_domain = category_ordered_index;
    }
#endif    
}


void XTreeDatabase::ExtendDomainWorker( PatternLink plink )
{
    // Extend locally first and then pass that into children.
    set<XLink> extra_xlinks = plink.GetChildAgent()->ExpandNormalDomain( *this, unordered_domain );    
    if( !extra_xlinks.empty() )
        TRACE("There are extra x domain elements for ")(plink)(":\n");
    for( XLink extra_xlink : extra_xlinks )
    {
        TRACEC(extra_xlink)("\n");
        AddAtRoot( STOP_IF_ALREADY_IN, extra_xlink ); // set to REQUIRE_SOLO to replicate #218
    }
    
    // Visit couplings repeatedly TODO union over couplings and
    // only recurse on last reaching.
    auto pq = plink.GetChildAgent()->GetPatternQuery();    
    for( PatternLink child_plink : pq->GetNormalLinks() )
    {
        ExtendDomainWorker( child_plink );
    }
    for( PatternLink child_plink : pq->GetAbnormalLinks() )
    {
        ExtendDomainWorker( child_plink );
    }
    for( PatternLink child_plink : pq->GetMultiplicityLinks() )
    {
        ExtendDomainWorker( child_plink );
    }
}


void XTreeDatabase::ExtendDomain( PatternLink plink )
{
    ExtendDomainWorker(plink);
#ifdef TRACE_X_TREE_DB_DELTAS
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain extended for ")(plink)(", new XLinks:\n")
             ( DifferenceOf(category_ordered_index, previous_category_ordered_domain) )
             ("\nRemoved XLinks:\n")
             ( DifferenceOf(previous_category_ordered_domain, category_ordered_index) )("\n");
        previous_category_ordered_domain = category_ordered_index;
    }
#endif

#ifdef TEST_RELATION_PROPERTIES_USING_DOMAIN    
    SimpleCompareRelation e;
    e.TestProperties( unordered_domain );
#endif
}


void XTreeDatabase::AddAtRoot( SubtreeMode mode, XLink root_xlink )
{
    // Bootstrap the recursive process with initial (root) values
    Row row;
    row.containment_context = Row::ROOT;
    row.my_container_front = root_xlink;
    row.my_container_back = root_xlink;
    
    NodeRow node_row;
    AddLink( mode, root_xlink, row, node_row );
}


void XTreeDatabase::AddLink( SubtreeMode mode, 
                            XLink xlink, 
                            Row &row,
                            NodeRow &node_row )
{
    // This will also prevent recursion into xlink
    if( mode==STOP_IF_ALREADY_IN && xlink_table.count(xlink) > 0 )
        return; // Terminate into the existing domain
    
    // Check for badness
    if( xlink_table.count(xlink) )
    {
		Row old_row = xlink_table.at(xlink);
		// remember that row is incomplete because 
		// we have not been able to fill everything in yet
		if( row.parent_xlink != old_row.parent_xlink )
		{
			ASSERT(false)
			      ("Rule #217 violation or cycle: node with child should have only one parent\n")
			      ("From parents: ")(row.parent_xlink)(" and ")(old_row.parent_xlink)
			      ("\nTo child: ")(xlink);
		}
		
		// Otherwise why did the parents not fail the check?
		ASSERTFAIL("Unknown trouble");				
	}
    
    // Update domains
    InsertSolo( unordered_domain, xlink );
    depth_first_ordered_index.push_back( xlink );
    category_ordered_index.insert( xlink );
    simple_compare_ordered_index.insert( xlink );
    
    DepthFirstOrderedIt it = depth_first_ordered_index.end();
    --it; // I know this is OK because we just pushed to ordered_domain
    row.depth_first_ordered_it = it;
    row.depth_first_ordinal = current_ordinal++;  
        
    node_row.parents.insert( xlink );    
        
    // Keep track of the last added on the way in.
    // AddChildren() may recuse back here and update last_link.
    last_xlink = xlink;
        
    // Recurse into our child nodes
    AddChildren( mode, xlink );

    // Grab last link that was added during unwind    
    row.last_descendant_xlink = last_xlink;
    
    // Add a row of x_tree_db
    InsertSolo( xlink_table, make_pair(xlink, row) );

	// Merge in the node row
	node_table[xlink.GetChildX()].Merge( node_row );	

    // Here, elements go into quotient set, but it does not 
    // uniquify: every link in the input X tree must appear 
    // separately in domain.
    (void)domain_extension_classes->Uniquify( xlink );    
}


void XTreeDatabase::AddChildren( SubtreeMode mode, XLink xlink )
{
    TreePtr<Node> x = xlink.GetChildX();
    vector< Itemiser::Element * > x_items = x->Itemise();
    for( Itemiser::Element *xe : x_items )
    {
        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(xe) )
            AddSequence( mode, x_seq, xlink );
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(xe) )
            AddCollection( mode, x_col, xlink );
        else if( TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(xe) )
            AddSingularNode( mode, p_x_singular, xlink );
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}


void XTreeDatabase::AddSingularNode( SubtreeMode mode, const TreePtrInterface *p_x_singular, XLink xlink )
{
    ASSERT( p_x_singular );
    // MakeValueArchetype() can generate nodes with NULL pointers (eg in PointerIs)
    // and these get into the domain even though they are not allowed in input trees.
    // In this case, stop recursing since there's no child to build x_tree_db for.    
    if( !*p_x_singular )
        return;
        
    TreePtr<Node> x = xlink.GetChildX();
    XLink child_xlink( x, p_x_singular );        
    TreePtr<Node> child_x = child_xlink.GetChildX();
    
    Row row;
    row.containment_context = Row::SINGULAR;
    row.parent_xlink = xlink;
    row.my_container_front = child_xlink;
    row.my_container_back = child_xlink;
    
    NodeRow node_row;
    set<const TreePtrInterface *> declared = x->GetDeclared();
    if( declared.count( p_x_singular ) > 0 )
		node_row.declarers.insert( child_xlink );
		
    AddLink( mode, child_xlink, row, node_row );
}


void XTreeDatabase::AddSequence( SubtreeMode mode, SequenceInterface *x_seq, XLink xlink )
{
    SequenceInterface::iterator xit_predecessor = x_seq->end();
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
		TreePtr<Node> x = xlink.GetChildX();
        XLink child_xlink( x, &*xit );
        TreePtr<Node> child_x = child_xlink.GetChildX();
        
        Row row;
        row.containment_context = Row::IN_SEQUENCE;
        row.parent_xlink = xlink;
        row.my_container_it = xit;        
        row.my_container_front = XLink( x, &x_seq->front() );
        row.my_container_back = XLink( x, &x_seq->back() );
        
        if( xit_predecessor != x_seq->end() )
            row.my_sequence_predecessor = XLink( x, &*xit_predecessor );

        SequenceInterface::iterator xit_successor = xit;
        ++xit_successor;
        if( xit_successor != x_seq->end() )
            row.my_sequence_successor = XLink( x, &*xit_successor );
        else
            row.my_sequence_successor = XLink::OffEndXLink;        
            
        NodeRow node_row;
        set<const TreePtrInterface *> declared = x->GetDeclared();
        if( declared.count( &*xit ) > 0 )
			node_row.declarers.insert( child_xlink );

        AddLink( mode, child_xlink, row, node_row );
        
        xit_predecessor = xit;
    }
}


void XTreeDatabase::AddCollection( SubtreeMode mode, CollectionInterface *x_col, XLink xlink )
{
    for( CollectionInterface::iterator xit = x_col->begin();
         xit != x_col->end();
         ++xit )
    {
		TreePtr<Node> x = xlink.GetChildX();
        XLink child_xlink( x, &*xit );
        TreePtr<Node> child_x = child_xlink.GetChildX();

        Row row;
        row.containment_context = Row::IN_COLLECTION;
        row.parent_xlink = xlink;
        row.my_container_it = xit;
        row.my_container_front = XLink( x, &*(x_col->begin()) );
        // Note: in real STL containers, one would use *(x_col->rbegin())
        row.my_container_back = XLink( x, &(x_col->back()) );
        
        NodeRow node_row;
        set<const TreePtrInterface *> declared = x->GetDeclared();
        if( declared.count( &*xit ) > 0 )
			node_row.declarers.insert( child_xlink );

        AddLink( mode, child_xlink, row, node_row );
    }
}


string XTreeDatabase::Row::GetTrace() const
{
    string s = "(";

    bool par = false;
    bool cont = false;
    bool idx = false;
    switch( containment_context )
    {
        case ROOT:
            s += "ROOT";
            break;
        case SINGULAR:
            s += "SINGULAR";
            par = true;
            break;
        case IN_SEQUENCE:
            s += "IN_SEQUENCE";
            par = cont = idx = true;
            break;
        case IN_COLLECTION:
            s += "IN_COLLECTION";
            par = cont = true;
            break;
    }    
    if( par )
        s += ", parent_xlink=" + Trace(parent_xlink);
    if( cont )
    {
        s += ", front=" + Trace(my_container_front);
        s += ", back=" + Trace(my_container_back);
    }
    if( idx )
        s += SSPrintf(", dfi=%d", depth_first_ordinal);
    s += ")";
    return s;
}


void XTreeDatabase::NodeRow::Merge( const NodeRow &nn )
{
	parents = UnionOf( parents, nn.parents );
	declarers = UnionOf( declarers, nn.declarers );
}


string XTreeDatabase::NodeRow::GetTrace() const
{
    string s = "(";
	
	s += " parents=" + Trace(parents);
	s += "declarers=" + Trace(declarers);
    
    s += ")";
    return s;
}


set<TreeKit::LinkInfo> XTreeDatabase::GetDeclarers( TreePtr<Node> node ) const
{
    set<LinkInfo> infos;
   
    if( node_table.count(node)==0 ) // not found
        throw UnknownNode();
        
    NodeRow nn = node_table.at(node);
    // Note that nn.declarers is "precise", i.e. the XLinks are the actual
    // declaring xlinks, not just arbitrary parent links to the declaree.
    // Also correct for parallel links where only some declare.
    for( XLink declarer_xlink : nn.declarers )
    {
        LinkInfo info;
        
        // first is TreePtr to the declarer node. Loses info about which 
        // link declared (in case of parallel links) but gets you the declarer node.
        info.first = xlink_table.at(declarer_xlink).parent_xlink.GetChildX();

        // second is TreePtrInterface * to the declarer's pointer to declaree
        // Retains precise info about which link.
        info.second = declarer_xlink.GetXPtr();
        
        infos.insert( info );
    }
    return infos;
}
