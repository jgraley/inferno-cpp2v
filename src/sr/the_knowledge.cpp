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
    
    int is = domain.size();
    ExtendDomain( root_plink );
    int es = domain.size();
    
    if( es > is )
        TRACE("Domain size %d -> %d\n", is, es);

    domain.insert(XLink::MMAX_Link);
    
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
    AddLink( mode, root_xlink, Nugget::ROOT );
}


void TheKnowledge::AddLink( SubtreeMode mode, 
                            XLink xlink, 
                            Nugget::Cadence cadence, 
                            XLink parent_xlink, 
                            const ContainerInterface *container, 
                            int index )
{
    INDENT(">");
    // This will also prevent recursion into xlink
    if( mode==STOP_IF_ALREADY_IN && domain.count(xlink) > 0 )
        return; // Terminate into the existing domain
    
    // Update domain
    InsertSolo( domain, xlink );
    
    // Add a nugget of knowledge
    Nugget nugget;
    nugget.parent_xlink = parent_xlink;
    nugget.cadence = cadence;
    nugget.container = container;
    nugget.index = index;
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
    AddLink( mode, child_xlink, Nugget::SINGULAR, xlink );
}


void TheKnowledge::AddSequence( SubtreeMode mode, const SequenceInterface *x_seq, XLink xlink )
{
    int index = 0;
    FOREACH( const TreePtrInterface &x, *x_seq )
    {
        XLink child_xlink( xlink.GetChildX(), &x );
        AddLink( mode, child_xlink, Nugget::IN_SEQUENCE, xlink, x_seq, index );
        index++;
    }
}


void TheKnowledge::AddCollection( SubtreeMode mode, const CollectionInterface *x_col, XLink xlink )
{
    FOREACH( const TreePtrInterface &x, *x_col )
    {
        XLink child_xlink( xlink.GetChildX(), &x );        
        AddLink( mode, child_xlink, Nugget::IN_COLLECTION, xlink, x_col );
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
        s += SSPrintf(", container=%p(%d)", container);
    if( idx )
        s += SSPrintf(", index=%d", index);
    s += ")";
    return s;
}


const TheKnowledge::Nugget &TheKnowledge::GetNugget(XLink xlink) const
{
    return nuggets.at(xlink);
}

