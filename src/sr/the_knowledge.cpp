#include "the_knowledge.hpp"
#include "sc_relation.hpp"
#include "agents/agent.hpp"
#include "vn_step.hpp"
#include "sym/expression.hpp"
#include "sym/predicate_operators.hpp"
#include "lacing.hpp"

using namespace SR;    

//#define TEST_RELATION_PROPERTIES_USING_DOMAIN
//#define TRACE_KNOWLEDGE_DELTAS

#ifdef TRACE_KNOWLEDGE_DELTAS
// Global because there are different knowledges owned by different SCR Engines
unordered_set<XLink> previous_unordered_domain;
#endif    


TheKnowledge::TheKnowledge( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    plan( clauses ),
    category_ordered_domain( plan.lacing )
{
}


TheKnowledge::Plan::Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    lacing( make_shared<Lacing>() )
{
    // Warning: there are a few places that declare an empty knowledge
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

    
XLink TheKnowledge::UniquifyDomainExtension( XLink xlink )
{
    ASSERT( xlink );
  
    // Don't worry about generated nodes that are already in 
    // the X tree (they had to have been found there after a
    // search). 
    if( unordered_domain.count(xlink) > 0 )
        return xlink;
        
    return domain_extension_classes->Uniquify( xlink ); 
}


XLink TheKnowledge::FindDomainExtension( XLink xlink ) const
{
    ASSERT( xlink );
    
    // Don't worry about generated nodes that are already in 
    // the X tree (they had to have been found there after a
    // search). 
    if( unordered_domain.count(xlink) > 0 )
        return xlink;
        
    return domain_extension_classes->Find( xlink ); 
}


const Lacing *TheKnowledge::GetLacing() const
{
    return plan.lacing.get();
}


TheKnowledge::CategoryRelation::CategoryRelation( shared_ptr<Lacing> lacing_ ) :
    lacing( lacing_ )
{
}


bool TheKnowledge::CategoryRelation::operator() (const XLink& x_link, const XLink& y_link) const
{
    TreePtr<Node> x = x_link.GetChildX();
    auto cat_x = TreePtr<CategoryMinimaxNode>::DynamicCast( x );
    TreePtr<Node> y = y_link.GetChildX();    
    auto cat_y = TreePtr<CategoryMinimaxNode>::DynamicCast( y );

    if( !cat_x && !cat_y )
        return lacing->IsIndexLess( x, y );    
   
    int xi, yi;
    if( cat_x )
        xi = cat_x->GetLacingIndex();
    else
        xi = lacing->GetIndexForNode( x );
    if( cat_y )
        yi = cat_y->GetLacingIndex();
    else
        yi = lacing->GetIndexForNode( y );
    return xi < yi;   
}


TheKnowledge::CategoryMinimaxNode::CategoryMinimaxNode( int lacing_index_ ) :
    lacing_index( lacing_index_ )
{
}
    

TheKnowledge::CategoryMinimaxNode::CategoryMinimaxNode() :
    lacing_index( 0 )
{
}
    

int TheKnowledge::CategoryMinimaxNode::GetLacingIndex() const
{
    return lacing_index;
}
 

string TheKnowledge::CategoryMinimaxNode::GetTrace() const
{
    return GetTypeName() + SSPrintf("(%d)", lacing_index);
}


const TheKnowledge::Nugget &TheKnowledge::GetNugget(XLink xlink) const
{
    ASSERT( xlink );
    ASSERT( HasNugget(xlink) )
          ("Knowledge: no nugget for ")(xlink)("\n")
          ("Nuggets: ")(nuggets);
    return nuggets.at(xlink);
}


bool TheKnowledge::HasNugget(XLink xlink) const
{
    ASSERT( xlink );
    return nuggets.count(xlink) > 0;
}


const TheKnowledge::NodeNugget &TheKnowledge::GetNodeNugget(TreePtr<Node> node) const
{
    ASSERT( node );
    ASSERT( HasNodeNugget(node) )
          ("Knowledge: no nugget for ")(node)("\n");
    //      ("Nuggets: ")(node_nuggets);
    return node_nuggets.at(node);
}


bool TheKnowledge::HasNodeNugget(TreePtr<Node> node) const
{
    ASSERT( node );
    return node_nuggets.count(node) > 0;
}


void TheKnowledge::Build( XLink root_xlink )
{      
	ASSERT( root_xlink );
	
    // Clear everything 
    unordered_domain.clear();
    depth_first_ordered_domain.clear();
    category_ordered_domain.clear();
    simple_compare_ordered_domain.clear();
    domain_extension_classes = make_shared<SimpleCompareQuotientSet>();
    nuggets.clear();
    node_nuggets.clear();    
    current_index = 0;
    
    AddAtRoot( REQUIRE_SOLO, root_xlink );
    
    AddAtRoot( REQUIRE_SOLO, XLink::MMAX_Link );
    AddAtRoot( REQUIRE_SOLO, XLink::OffEndXLink );
        
#ifdef TRACE_KNOWLEDGE_DELTAS
    TRACE("Knowledge regenerated: new XLinks:\n")
         ( DifferenceOf(unordered_domain, previous_unordered_domain) )
         ("\nRemoved XLinks:\n")
         ( DifferenceOf(previous_unordered_domain, unordered_domain) )("\n");
    previous_unordered_domain = unordered_domain;
#endif    
}


void TheKnowledge::ExtendDomain( PatternLink plink )
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
        ExtendDomain( child_plink );
    }
    for( PatternLink child_plink : pq->GetAbnormalLinks() )
    {
        ExtendDomain( child_plink );
    }
    for( PatternLink child_plink : pq->GetMultiplicityLinks() )
    {
        ExtendDomain( child_plink );
    }
    
#ifdef TRACE_KNOWLEDGE_DELTAS
    TRACE("Knowledge regenerated: new XLinks:\n")
         ( DifferenceOf(unordered_domain, previous_unordered_domain) )
         ("\nRemoved XLinks:\n")
         ( DifferenceOf(previous_unordered_domain, unordered_domain) )("\n");
    previous_unordered_domain = unordered_domain;
#endif

#ifdef TEST_RELATION_PROPERTIES_USING_DOMAIN    
    SimpleCompareRelation e;
    e.TestProperties( unordered_domain );
#endif
}


void TheKnowledge::AddAtRoot( SubtreeMode mode, XLink root_xlink )
{
    // Bootstrap the recursive process with initial (root) values
    Nugget nugget;
    nugget.containment_context = Nugget::ROOT;
    nugget.my_container_front = root_xlink;
    nugget.my_container_back = root_xlink;
    
    AddLink( mode, root_xlink, nugget );
}


void TheKnowledge::AddLink( SubtreeMode mode, 
                            XLink xlink, 
                            Nugget nugget )
{
    // This will also prevent recursion into xlink
    if( mode==STOP_IF_ALREADY_IN && nuggets.count(xlink) > 0 )
        return; // Terminate into the existing domain
    
    // Check for badness
    if( nuggets.count(xlink) )
    {
		Nugget old_nugget = nuggets.at(xlink);
		// remember that nugget is incomplete because 
		// we have not been able to fill everything in yet
		if( nugget.parent_xlink != old_nugget.parent_xlink )
		{
			ASSERT(false)
			      ("Rule #217 violation or cycle: node with child should have only one parent\n")
			      ("From parents: ")(nugget.parent_xlink)(" and ")(old_nugget.parent_xlink)
			      ("\nTo child: ")(xlink);
		}
		
		// Otherwise why did the parents not fail the check?
		ASSERTFAIL("Unknown trouble");				
	}
    
    // Update domains
    InsertSolo( unordered_domain, xlink );
    depth_first_ordered_domain.push_back( xlink );
    category_ordered_domain.insert( xlink );
    simple_compare_ordered_domain.insert( xlink );
    
    DepthFirstOrderedIt it = depth_first_ordered_domain.end();
    --it; // I know this is OK because we just pushed to ordered_domain
    nugget.depth_first_ordered_it = it;
    nugget.depth_first_index = current_index++;  
        
    // Keep track of the last added on the way in.
    // AddChildren() may recuse back here and update last_link.
    last_xlink = xlink;
        
    // Recurse into our child nodes
    AddChildren( mode, xlink );

    // Grab last link that was added during unwind    
    nugget.last_descendant_xlink = last_xlink;
    
    // Add a nugget of knowledge
    InsertSolo( nuggets, make_pair(xlink, nugget) );

    // Here, elements go into quotient set, but it does not 
    // uniquify: every link in the input X tree must appear 
    // separately in domain.
    (void)domain_extension_classes->Uniquify( xlink );    
}


void TheKnowledge::AddChildren( SubtreeMode mode, XLink xlink )
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


void TheKnowledge::AddSingularNode( SubtreeMode mode, const TreePtrInterface *p_x_singular, XLink xlink )
{
    ASSERT( p_x_singular );
    // MakeValueArchetype() can generate nodes with NULL pointers (eg in PointerIs)
    // and these get into the domain even though they are not allowed in input trees.
    // In this case, stop recursing since there's no child to build knowledge for.    
    if( !*p_x_singular )
        return;
        
    TreePtr<Node> x = xlink.GetChildX();
    XLink child_xlink( x, p_x_singular );        
    TreePtr<Node> child_x = child_xlink.GetChildX();
    
    Nugget nugget;
    nugget.containment_context = Nugget::SINGULAR;
    nugget.parent_xlink = xlink;
    nugget.my_container_front = child_xlink;
    nugget.my_container_back = child_xlink;
    
    node_nuggets[child_x].parents.insert( child_xlink );
    set<const TreePtrInterface *> declared = x->GetDeclared();
    if( declared.count( p_x_singular ) > 0 )
		node_nuggets[child_x].declarers.insert( child_xlink );
		
    AddLink( mode, child_xlink, nugget );
}


void TheKnowledge::AddSequence( SubtreeMode mode, SequenceInterface *x_seq, XLink xlink )
{
    SequenceInterface::iterator xit_predecessor = x_seq->end();
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
		TreePtr<Node> x = xlink.GetChildX();
        XLink child_xlink( x, &*xit );
        TreePtr<Node> child_x = child_xlink.GetChildX();
        
        Nugget nugget;
        nugget.containment_context = Nugget::IN_SEQUENCE;
        nugget.parent_xlink = xlink;
        nugget.my_container_it = xit;        
        nugget.my_container_front = XLink( x, &x_seq->front() );
        nugget.my_container_back = XLink( x, &x_seq->back() );
        
        if( xit_predecessor != x_seq->end() )
            nugget.my_sequence_predecessor = XLink( x, &*xit_predecessor );

        SequenceInterface::iterator xit_successor = xit;
        ++xit_successor;
        if( xit_successor != x_seq->end() )
            nugget.my_sequence_successor = XLink( x, &*xit_successor );
        else
            nugget.my_sequence_successor = XLink::OffEndXLink;        
            
        node_nuggets[child_x].parents.insert( child_xlink );
        set<const TreePtrInterface *> declared = x->GetDeclared();
        if( declared.count( &*xit ) > 0 )
			node_nuggets[child_x].declarers.insert( child_xlink );
			
        AddLink( mode, child_xlink, nugget );
        
        xit_predecessor = xit;
    }
}


void TheKnowledge::AddCollection( SubtreeMode mode, CollectionInterface *x_col, XLink xlink )
{
    for( CollectionInterface::iterator xit = x_col->begin();
         xit != x_col->end();
         ++xit )
    {
		TreePtr<Node> x = xlink.GetChildX();
        XLink child_xlink( x, &*xit );
        TreePtr<Node> child_x = child_xlink.GetChildX();

        Nugget nugget;
        nugget.containment_context = Nugget::IN_COLLECTION;
        nugget.parent_xlink = xlink;
        nugget.my_container_it = xit;
        nugget.my_container_front = XLink( x, &*(x_col->begin()) );
        // Note: in real STL containers, one would use *(x_col->rbegin())
        nugget.my_container_back = XLink( x, &(x_col->back()) );
        
		node_nuggets[child_x].parents.insert( child_xlink );
        set<const TreePtrInterface *> declared = x->GetDeclared();
        if( declared.count( &*xit ) > 0 )
			node_nuggets[child_x].declarers.insert( child_xlink );

        AddLink( mode, child_xlink, nugget );
    }
}


string TheKnowledge::Nugget::GetTrace() const
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
        s += SSPrintf(", dfi=%d", depth_first_index);
    s += ")";
    return s;
}


string TheKnowledge::NodeNugget::GetTrace() const
{
    string s = "(";
	
	s += "declarers=" + Trace(declarers);
    
    s += ")";
    return s;
}


set<TreeKit::LinkInfo> TheKnowledge::GetDeclarers( TreePtr<Node> node ) const
{
    set<LinkInfo> infos;
   
    if( node_nuggets.count(node)==0 ) // not found
        throw UnknownNode();
        
    NodeNugget nn = node_nuggets.at(node);
    // Note that nn.declarers is "precise", i.e. the XLinks are the actual
    // declaring xlinks, not just arbitrary parent links to the declaree.
    // Also correct for parallel links where only some declare.
    for( XLink declarer_xlink : nn.declarers )
    {
        LinkInfo info;
        
        // first is TreePtr to the declarer node. Loses info about which 
        // link declared (in case of parallel links) but gets you the declarer node.
        info.first = nuggets.at(declarer_xlink).parent_xlink.GetChildX();

        // second is TreePtrInterface * to the declarer's pointer to declaree
        // Retains precise info about which link.
        info.second = declarer_xlink.GetXPtr();
        
        infos.insert( info );
    }
    return infos;
}
