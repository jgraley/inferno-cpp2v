#include "tables.hpp"
#include "../sc_relation.hpp"


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


DBWalk::Actions Tables::GetActions()
{
	DBWalk::Actions actions;

	actions.domain_in_is_ok = [&](const DBWalk::WalkInfo &walk_info) -> bool
	{
		return domain->unordered_domain.count( walk_info.xlink ) == 0;
	};
	
	actions.domain_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		// ----------------- Update domain
		InsertSolo( domain->unordered_domain, walk_info.xlink );
		
		// Here, elements go into quotient set, but it does not 
		// uniquify: every link in the input X tree must appear 
		// separately in domain.
		(void)domain->domain_extension_classes->Uniquify( walk_info.xlink );    
	};

	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> Indexes::DepthFirstOrderedIt
	{
		// ----------------- Update indices
		indexes->depth_first_ordered_index.push_back( walk_info.xlink );
		Indexes::DepthFirstOrderedIt it = indexes->depth_first_ordered_index.end();
		--it; // I know this is OK because we just pushed to depth_first_ordered_index

		indexes->category_ordered_index.insert( walk_info.xlink );

		indexes->simple_compare_ordered_index.insert( walk_info.xlink );
		
		return it;
	};

	actions.xlink_row_in = [&](const DBWalk::WalkInfo &walk_info, 
	                           Indexes::DepthFirstOrderedIt df_it)
	{
		// ----------------- Generate row
		Row row;        
		row.containment_context = walk_info.context;
		switch( row.containment_context )
		{
		case DBWalk::ROOT:
		{
			row.my_container_front = walk_info.xlink;
			row.my_container_back = walk_info.xlink;
			break;
		}	
		case DBWalk::SINGULAR:
		{
			row.parent_xlink = walk_info.parent_xlink;
			row.my_container_front = walk_info.xlink;
			row.my_container_back = walk_info.xlink;
			break;
		}
		case DBWalk::IN_SEQUENCE:
		{
			TreePtr<Node> parent_x = walk_info.parent_xlink.GetChildX();

			row.parent_xlink = walk_info.parent_xlink;
			row.my_container_it = walk_info.xit;        
			row.my_container_front = XLink( parent_x, &walk_info.p_xcon->front() );
			row.my_container_back = XLink( parent_x, &walk_info.p_xcon->back() );
			
			if( walk_info.xit_predecessor != walk_info.p_xcon->end() )
				row.my_sequence_predecessor = XLink( parent_x, &*walk_info.xit_predecessor );

			SequenceInterface::iterator xit_successor = walk_info.xit;
			++xit_successor;
			if( xit_successor != walk_info.p_xcon->end() )
				row.my_sequence_successor = XLink( parent_x, &*xit_successor );
			else
				row.my_sequence_successor = XLink::OffEndXLink;        
			break;
		}
		case DBWalk::IN_COLLECTION:
		{
			TreePtr<Node> parent_x = walk_info.parent_xlink.GetChildX();

			row.parent_xlink = walk_info.parent_xlink;
			row.my_container_it = walk_info.xit;
			row.my_container_front = XLink( parent_x, &*(walk_info.p_xcon->begin()) );
			// Note: in real STL containers, one would use *(x_col->rbegin())
			row.my_container_back = XLink( parent_x, &(walk_info.p_xcon->back()) );
			break;
		}
		}
			
		row.depth_first_ordered_it = df_it;
		row.depth_first_ordinal = current_ordinal++;  
			
		// Check for badness
		if( xlink_table.count(walk_info.xlink) )
		{
			Row old_row = xlink_table.at(walk_info.xlink);
			// remember that row is incomplete because 
			// we have not been able to fill everything in yet
			if( row.parent_xlink != old_row.parent_xlink )
			{
				ASSERT(false)
					  ("Rule #217 violation or cycle: node with child should have only one parent\n")
					  ("From parents: ")(row.parent_xlink)(" and ")(old_row.parent_xlink)
					  ("\nTo child: ")(walk_info.xlink);
			}
			
			// Otherwise why did the parents not fail the check?
			ASSERTFAIL("Unknown trouble");				
		}
		
		// Keep track of the last added on the way in.
		// AddChildren() may recuse back here and update last_link.
		last_xlink = walk_info.xlink;

		// Add a row of x_tree_db
		InsertSolo( xlink_table, make_pair(walk_info.xlink, row) );
	};

	actions.node_row_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		// ----------------- Generate node row
		NodeRow node_row;
		switch( walk_info.context )
		{
			
		case DBWalk::ROOT:
		{
			break;
		}	
		case DBWalk::SINGULAR:
		case DBWalk::IN_SEQUENCE:
		case DBWalk::IN_COLLECTION:
		{
			TreePtr<Node> parent_x = walk_info.parent_xlink.GetChildX();
			set<const TreePtrInterface *> declared = parent_x->GetDeclared();
			if( declared.count( walk_info.p_x ) > 0 )
				node_row.declarers.insert( walk_info.xlink );
			break;
		}
		}
		node_row.parents.insert( walk_info.xlink );    

		// Merge in the node row
		node_table[walk_info.xlink.GetChildX()].Merge( node_row );			
	};
	
	actions.xlink_row_out = [&](const DBWalk::WalkInfo &walk_info)
	{
		// ----------------- Generate row unwind
		// Grab last link that was added during unwind    
		xlink_table.at(walk_info.xlink).last_descendant_xlink = last_xlink;
	};
	
	return actions;
}


void Tables::FullBuild( XLink root_xlink )
{
    current_ordinal = 0;
    
    //AddAtRoot( REQUIRE_SOLO, root_xlink );
    
    //AddAtRoot( REQUIRE_SOLO, XLink::MMAX_Link );
    //AddAtRoot( REQUIRE_SOLO, XLink::OffEndXLink );
    
    db_walker.FullWalk( GetActions(), root_xlink );
}


Domain::OnExtraXLinkFunction Tables::GetOnExtraXLinkFunction()
{
	return [&](XLink extra_xlink)
	{
        TRACEC(extra_xlink)("\n");
        // set to REQUIRE_SOLO to replicate #218
		//AddAtRoot( Tables::STOP_IF_ALREADY_IN, extra_xlink ); 
		db_walker.ExtraXLinkWalk( GetActions(), extra_xlink );
    };
}


void Tables::AddAtRoot( SubtreeMode mode, XLink root_xlink )
{
    // Bootstrap the recursive process with initial (root) values
    AddLink( mode, 
             { DBWalk::ROOT, 
               root_xlink, 
               XLink(), 
               nullptr,
			   ContainerInterface::iterator(),
               ContainerInterface::iterator(),
               nullptr } );
}


void Tables::AddSingularNode( SubtreeMode mode, const TreePtrInterface *p_x_singular, XLink xlink )
{
    ASSERT( p_x_singular );
    
    // MakeValueArchetype() can generate nodes with NULL pointers (eg in PointerIs)
    // and these get into the domain even though they are not allowed in input trees.
    // In this case, stop recursing since there's no child to build a row for.    
    if( !*p_x_singular )
        return;
        
    TreePtr<Node> x = xlink.GetChildX();
    XLink child_xlink( x, p_x_singular );        
    
    AddLink( mode, 
             { DBWalk::SINGULAR, 
               child_xlink, 
               xlink, 
               nullptr,
			   ContainerInterface::iterator(),
               ContainerInterface::iterator(),
               p_x_singular } );
}


void Tables::AddSequence( SubtreeMode mode, SequenceInterface *x_seq, XLink xlink )
{
	TreePtr<Node> x = xlink.GetChildX();
    SequenceInterface::iterator xit_predecessor = x_seq->end();
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit );

        AddLink( mode, 
                 { DBWalk::IN_SEQUENCE, 
                   child_xlink, 
                   xlink,
                   x_seq,
                   xit_predecessor,
                   xit,
                   &*xit } );
        
        xit_predecessor = xit;
    }
}


void Tables::AddCollection( SubtreeMode mode, CollectionInterface *x_col, XLink xlink )
{
	TreePtr<Node> x = xlink.GetChildX();
    for( CollectionInterface::iterator xit = x_col->begin();
         xit != x_col->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit );

        AddLink( mode, 
                 { DBWalk::IN_COLLECTION, 
                   child_xlink, 
                   xlink,
                   x_col, 
				   ContainerInterface::iterator(),
                   xit,
                   &*xit } );
    }
}


void Tables::AddLink( SubtreeMode mode, 
                      const WalkInfo &walk_info )
{
    // This will also prevent recursion into xlink
    if( mode==STOP_IF_ALREADY_IN && xlink_table.count(walk_info.xlink) > 0 )
        return; // Terminate into the existing domain
        

    // ----------------- Recurse

    // Recurse into our child nodes
    AddChildren( mode, walk_info.xlink );

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


string Tables::Row::GetTrace() const
{
    string s = "(";

    bool par = false;
    bool cont = false;
    bool idx = false;
    switch( containment_context )
    {
        case DBWalk::ROOT:
            s += "ROOT";
            break;
        case DBWalk::SINGULAR:
            s += "SINGULAR";
            par = true;
            break;
        case DBWalk::IN_SEQUENCE:
            s += "IN_SEQUENCE";
            par = cont = idx = true;
            break;
        case DBWalk::IN_COLLECTION:
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

