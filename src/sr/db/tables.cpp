#include "tables.hpp"
#include "../sc_relation.hpp"
#include "../agents/agent.hpp"
#include "../vn_step.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"
#include "lacing.hpp"

using namespace SR;    


Tables::Tables( shared_ptr<Indexes> indexes_,
                shared_ptr<Domain> domain_ ) :
	indexes( indexes_ ),
	domain( domain_ )
{
}


const Tables::Row &Tables::GetRow(XLink xlink) const
{
    ASSERT( xlink );
    ASSERT( HasRow(xlink) )
          ("X tree database: no row for ")(xlink)("\n")
          ("Rows: ")(xlink_table);
    return xlink_table.at(xlink);
}


bool Tables::HasRow(XLink xlink) const
{
    ASSERT( xlink );
    return xlink_table.count(xlink) > 0;
}


const Tables::NodeRow &Tables::GetNodeRow(TreePtr<Node> node) const
{
    ASSERT( node );
    ASSERT( HasNodeRow(node) )
          ("X tree database: no node row for ")(node)("\n")
          ("Node xlink_table: ")(node_table);
    return node_table.at(node);
}


bool Tables::HasNodeRow(TreePtr<Node> node) const
{
    ASSERT( node );
    return node_table.count(node) > 0;
}


void Tables::FullBuild( XLink root_xlink )
{
    current_ordinal = 0;
    
    AddAtRoot( REQUIRE_SOLO, root_xlink );
    
    AddAtRoot( REQUIRE_SOLO, XLink::MMAX_Link );
    AddAtRoot( REQUIRE_SOLO, XLink::OffEndXLink );
}


void Tables::AddAtRoot( SubtreeMode mode, XLink root_xlink )
{
    // Bootstrap the recursive process with initial (root) values
    Row row;
    row.containment_context = Row::ROOT;
    row.my_container_front = root_xlink;
    row.my_container_back = root_xlink;
    
    NodeRow node_row;
    AddLink( mode, root_xlink, row, node_row );
}


void Tables::AddLink( SubtreeMode mode, 
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
    InsertSolo( domain->unordered_domain, xlink );
    indexes->depth_first_ordered_index.push_back( xlink );
    indexes->category_ordered_index.insert( xlink );
    indexes->simple_compare_ordered_index.insert( xlink );
    
    Indexes::DepthFirstOrderedIt it = indexes->depth_first_ordered_index.end();
    --it; // I know this is OK because we just pushed to depth_first_ordered_index
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
    (void)domain->domain_extension_classes->Uniquify( xlink );    
}


void Tables::AddChildren( SubtreeMode mode, XLink xlink )
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


void Tables::AddSingularNode( SubtreeMode mode, const TreePtrInterface *p_x_singular, XLink xlink )
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


void Tables::AddSequence( SubtreeMode mode, SequenceInterface *x_seq, XLink xlink )
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


void Tables::AddCollection( SubtreeMode mode, CollectionInterface *x_col, XLink xlink )
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


string Tables::Row::GetTrace() const
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


void Tables::NodeRow::Merge( const NodeRow &nn )
{
	parents = UnionOf( parents, nn.parents );
	declarers = UnionOf( declarers, nn.declarers );
}


string Tables::NodeRow::GetTrace() const
{
    string s = "(";
	
	s += " parents=" + Trace(parents);
	s += "declarers=" + Trace(declarers);
    
    s += ")";
    return s;
}

