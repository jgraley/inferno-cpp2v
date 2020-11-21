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
    parents.clear();
    
    AddSubtreeToDomain( XLink(), root_xlink, REQUIRE_SOLO );
    
    domain.insert(XLink::MMAX_Link);

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
    for( XLink e : extra )
    {
        TRACE("Extra item for ")(plink)(" is ")(e)("\n");
        AddSubtreeToDomain( XLink(), e, STOP_IF_ALREADY_IN ); // set to REQUIRE_SOLO to replicate #218
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


void TheKnowledge::AddSubtreeToDomain( XLink parent_xlink, XLink xlink, SubtreeMode mode )
{
    // This will also prevent recursion into xlink
    if( mode==STOP_IF_ALREADY_IN && domain.count(xlink) > 0 )
        return; // Terminate into the existing domain
    
    InsertSolo( domain, xlink );
    if( parent_xlink )
        InsertSolo( parents, make_pair(xlink, parent_xlink) );

    // Here, elements go into quotient set, but it does not 
    // uniquify: every link in the input X tree must appear 
    // separately in domain.
    (void)domain_extension_classes->Uniquify( xlink );

    // Put all the nodes in the X tree into the domain
	FlattenNode fx( xlink.GetChildX() ); 
	for( FlattenNode::iterator fx_it=fx.begin(); fx_it!=fx.end(); ++fx_it )
    {
        XLink child_xlink( xlink.GetChildX(), &*fx_it );        
                
        AddSubtreeToDomain( xlink, child_xlink, mode );
    }
}
