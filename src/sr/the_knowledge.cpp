#include "the_knowledge.hpp"
#include "equivalence.hpp"
#include "agents/agent.hpp"

using namespace SR;    

//#define TEST_RELATION_PROPERTIES_USING_DOMAIN

void TheKnowledge::Build( PatternLink root_plink, XLink root_xlink )
{
    INDENT("K");
    DetermineDomain( root_plink, root_xlink );
}

    
void TheKnowledge::DetermineDomain( PatternLink root_plink, XLink root_xlink )
{   
    // Both should be cleared together
    domain.clear();
    domain_extension_classes = make_shared<QuotientSet>();
    nuggets.clear();
    
    AddSubtree( REQUIRE_SOLO, root_xlink );
    
    AddSubtree( REQUIRE_SOLO, XLink::MMAX_Link );
    
    int is = domain.size();
    ExtendDomain( root_plink );
    int es = domain.size();
    
    if( es > is )
        TRACE("Domain size %d -> %d\n", is, es);

    
#ifdef TEST_RELATION_PROPERTIES_USING_DOMAIN    
    EquivalenceRelation e;
    e.TestProperties( domain );
#endif
}


void TheKnowledge::ExtendDomain( PatternLink plink )
{
    // Extend locally first and then pass that into children.

    unordered_set<XLink> extra = plink.GetChildAgent()->ExpandNormalDomain( domain );          
    for( XLink exlink : extra )
    {
        TRACE("Extra item for ")(plink)(" is ")(exlink)("\n");
        AddSubtree( STOP_IF_ALREADY_IN, exlink ); // set to REQUIRE_SOLO to replicate #218
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


void TheKnowledge::AddSubtree( SubtreeMode mode, XLink root_xlink )
{
    // Bootstrap the recursive process with initial (root) values
    Nugget nugget;
    nugget.cadence = Nugget::ROOT;
    AddLink( mode, root_xlink, nugget );
}


void TheKnowledge::AddLink( SubtreeMode mode, 
                            XLink xlink, 
                            Nugget nugget )
{
    INDENT(">");
    // This will also prevent recursion into xlink
    if( mode==STOP_IF_ALREADY_IN && domain.count(xlink) > 0 )
        return; // Terminate into the existing domain
    
    // Update domain
    InsertSolo( domain, xlink );
    
    // Add a nugget of knowledge
    InsertSolo( nuggets, make_pair(xlink, nugget) );

    // Here, elements go into quotient set, but it does not 
    // uniquify: every link in the input X tree must appear 
    // separately in domain.
    (void)domain_extension_classes->Uniquify( xlink );
    
    // Recurse into our child nodes
    AddChildren( mode, xlink );
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
        else if( TreePtrInterface *x_sing = dynamic_cast<TreePtrInterface *>(xe) )
            AddSingularNode( mode, x_sing, xlink );
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}


void TheKnowledge::AddSingularNode( SubtreeMode mode, const TreePtrInterface *x_sing, XLink xlink )
{
    XLink child_xlink( xlink.GetChildX(), x_sing );        
    Nugget nugget;
    nugget.cadence = Nugget::SINGULAR;
    nugget.parent_xlink = xlink;
    AddLink( mode, child_xlink, nugget );
}


void TheKnowledge::AddSequence( SubtreeMode mode, SequenceInterface *x_seq, XLink xlink )
{
    Nugget::IndexType index = 0;
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
        XLink child_xlink( xlink.GetChildX(), &*xit );
        Nugget nugget;
        nugget.cadence = Nugget::IN_SEQUENCE;
        nugget.parent_xlink = xlink;
        nugget.container = x_seq;
        nugget.iterator = xit;
        nugget.index = index;  
        AddLink( mode, child_xlink, nugget );
        index++;
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
        nugget.cadence = Nugget::IN_COLLECTION;
        nugget.parent_xlink = xlink;
        nugget.container = x_col;
        nugget.iterator = xit;
        AddLink( mode, child_xlink, nugget );
    }
}


string TheKnowledge::Nugget::GetTrace() const
{
    string s = "(";

    bool par = false;
    bool cont = false;
    bool idx = false;
    switch( cadence )
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
        s += SSPrintf(", container=%p(%d)", container, container->size());
        s += "@" + (iterator==container->end() ? string("END") : Trace(*iterator));
    }
    if( idx )
        s += SSPrintf(", index=%d", index);
    s += ")";
    return s;
}


const TheKnowledge::Nugget &TheKnowledge::GetNugget(XLink xlink) const
{
    ASSERT( nuggets.count(xlink) > 0 )
          ("Knowledge: no nugget for ")(xlink)("\n")
          ("Nuggets: ")(nuggets);
    return nuggets.at(xlink);
}

