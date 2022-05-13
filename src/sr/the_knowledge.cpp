#include "the_knowledge.hpp"
#include "sc_relation.hpp"
#include "agents/agent.hpp"

using namespace SR;    

//#define TEST_RELATION_PROPERTIES_USING_DOMAIN
//#define TRACE_KNOWLEDGE_DELTAS


#ifdef TRACE_KNOWLEDGE_DELTAS
// Global because there are different knowledges owned by different SCR Engines
unordered_set<XLink> previous_unordered_domain;
#endif    


void TheKnowledge::Build( PatternLink root_plink, XLink root_xlink )
{
    DetermineDomain( root_plink, root_xlink );
}


void TheKnowledge::Clear()
{
    unordered_domain.clear();
    depth_first_ordered_domain.clear();
    simple_compare_ordered_domain.clear();
    nuggets.clear();
    if( domain_extension_classes )
        domain_extension_classes->Clear();
}

    
void TheKnowledge::DetermineDomain( PatternLink root_plink, XLink root_xlink )
{      
    // Both should be cleared together
    unordered_domain.clear();
    depth_first_ordered_domain.clear();
    simple_compare_ordered_domain.clear();
    domain_extension_classes = make_shared<SimpleCompareQuotientSet>();
    nuggets.clear();
    current_index = 0;
    
    AddAtRoot( REQUIRE_SOLO, root_xlink );
    
    AddAtRoot( REQUIRE_SOLO, XLink::MMAX_Link );
    AddAtRoot( REQUIRE_SOLO, XLink::OffEndXLink );
    
    int is = nuggets.size();
    ExtendDomain( root_plink );
    int es = nuggets.size();
    
    if( es > is )
        TRACE("Knowledge size %d -> %d\n", is, es);
    
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


void TheKnowledge::ExtendDomain( PatternLink plink )
{
    // Extend locally first and then pass that into children.
    set<XLink> extra_xlinks = plink.GetChildAgent()->ExpandNormalDomain( unordered_domain );    
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
    
    // Update domain 
    InsertSolo( unordered_domain, xlink );
    depth_first_ordered_domain.push_back(xlink);
    simple_compare_ordered_domain.insert(xlink);
    
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
    vector< Itemiser::Element * > x_memb = xlink.GetChildX()->Itemise();
    for( Itemiser::Element *xe : x_memb )
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
        
    XLink child_xlink( xlink.GetChildX(), p_x_singular );        
    Nugget nugget;
    nugget.containment_context = Nugget::SINGULAR;
    nugget.parent_xlink = xlink;
    nugget.my_container_front = child_xlink;
    nugget.my_container_back = child_xlink;
    AddLink( mode, child_xlink, nugget );
}


void TheKnowledge::AddSequence( SubtreeMode mode, SequenceInterface *x_seq, XLink xlink )
{
    SequenceInterface::iterator xit_predecessor = x_seq->end();
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
        XLink child_xlink( xlink.GetChildX(), &*xit );
        Nugget nugget;
        nugget.containment_context = Nugget::IN_SEQUENCE;
        nugget.parent_xlink = xlink;
        nugget.my_container_it = xit;        
        nugget.my_container_front = XLink( xlink.GetChildX(), &x_seq->front() );
        nugget.my_container_back = XLink( xlink.GetChildX(), &x_seq->back() );
        
        if( xit_predecessor != x_seq->end() )
            nugget.my_sequence_predecessor = XLink( xlink.GetChildX(), &*xit_predecessor );

        SequenceInterface::iterator xit_successor = xit;
        ++xit_successor;
        if( xit_successor != x_seq->end() )
            nugget.my_sequence_successor = XLink( xlink.GetChildX(), &*xit_successor );
        else
            nugget.my_sequence_successor = XLink::OffEndXLink;        
            
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
        XLink child_xlink( xlink.GetChildX(), &*xit );        
        Nugget nugget;
        nugget.containment_context = Nugget::IN_COLLECTION;
        nugget.parent_xlink = xlink;
        nugget.my_container_it = xit;
        nugget.my_container_front = XLink( xlink.GetChildX(), &*(x_col->begin()) );
        // Note: in real STL containers, one would use *(x_col->rbegin())
        nugget.my_container_back = XLink( xlink.GetChildX(), &(x_col->back()) );
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


const TheKnowledge::Nugget &TheKnowledge::GetNugget(XLink xlink) const
{
    ASSERT( xlink );
    ASSERT( HasNugget(xlink) )
          ("Knowledge: no nugget for ")(xlink)("\n");
    //      ("Nuggets: ")(nuggets);
    return nuggets.at(xlink);
}


bool TheKnowledge::HasNugget(XLink xlink) const
{
    ASSERT( xlink );
    return nuggets.count(xlink) > 0;
}


bool TheKnowledge::HasNuggetOrIsSubcontainer(XLink xlink) const
{
    ASSERT( xlink );
    return TreePtr<SubContainer>::DynamicCast(xlink.GetChildX()) || nuggets.count(xlink) > 0;
}

