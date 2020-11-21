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
    
    // Put all the nodes in the X tree into the domain
	Walk wx( root_xlink.GetChildX() ); 
	for( Walk::iterator wx_it=wx.begin(); wx_it!=wx.end(); ++wx_it )
    {
        XLink xlink = XLink::FromWalkIterator( wx_it, root_xlink );        

        // Here, elements go into quotient set, but it does not 
        // uniquify: every link in the input X tree must appear 
        // separately in domain.
        domain.insert( xlink );
        (void)domain_extension_classes->Uniquify( xlink );
        
        if( xlink != root_xlink )
        {
            XLink parent_xlink = XLink::FromWalkIterator( wx_it, root_xlink, 1 );        
            InsertSolo( parents, make_pair(xlink, parent_xlink) );
        }
        
        TRACEC("Added ")(xlink)("\n");
    }
    domain.insert(XLink::MMAX_Link);

    int is = domain.size();
    ExtendDomain( root_plink );
    int es = domain.size();
    
    if( es > is )
    {
        TRACE("Domain size %d -> %d\n", is, es);
    }
    
#ifdef TEST_RELATION_PROPERTIES_USING_DOMAIN    
    EquivalenceRelation e;
    e.TestProperties( domain );
#endif
}


void TheKnowledge::ExtendDomain( PatternLink plink )
{
    // Extend locally first and then pass that into children.

    unordered_set<XLink> extra = plink.GetChildAgent()->ExpandNormalDomain( domain );          
    if( !extra.empty() )
        TRACEC("Extra domain for ")(plink)(" is ")(extra)("\n");
    domain = UnionOf( domain, extra );
    
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

