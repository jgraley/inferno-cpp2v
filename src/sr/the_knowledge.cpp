#include "the_knowledge.hpp"
#include "sc_relation.hpp"
#include "agents/agent.hpp"
#include "vn_transformation.hpp"
#include "sym/expression.hpp"
#include "sym/predicate_operators.hpp"
#include "../helpers/simple_compare.hpp"

using namespace SR;    

//#define TEST_RELATION_PROPERTIES_USING_DOMAIN
//#define TRACE_KNOWLEDGE_DELTAS


#ifdef TRACE_KNOWLEDGE_DELTAS
// Global because there are different knowledges owned by different SCR Engines
unordered_set<XLink> previous_unordered_domain;
#endif    


TheKnowledge::TheKnowledge( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    plan( clauses )
{
}


TheKnowledge::Plan::Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses_ ) :
    simple_compare( make_shared<SimpleCompare>() ),
    clauses( clauses_ ),
    categories( *simple_compare )
{
    // Warning: there are a few places that declare an empty knowledge
    if( clauses.empty() )
        return;
    
    // Extract all the non-final archetypes from the KindOfOperator nodes 
    // into a set so that they are uniqued by SimpleCompare equality. These
    // are the categories.
    for( shared_ptr<SYM::BooleanExpression> clause : clauses )
    {
        clause->ForDepthFirstWalk( [&](const SYM::Expression *expr)
        {
            if( auto ko_expr = dynamic_cast<const SYM::KindOfOperator *>(expr) )
            { 
                TreePtr<Node> arch = ko_expr->GetArchetypeNode();
                if( !arch->IsFinal() )
                    categories.insert( arch );
            }
        } );
    }
    
    // Find the set of strict/non-strict super/sub-categories for each category.
    for( TreePtr<Node> arch : categories )
    {
        cats_to_nonstrict_supercats.emplace( std::piecewise_construct,
                                        std::forward_as_tuple(arch),
                                        std::forward_as_tuple(*simple_compare) );
        cats_to_nonstrict_subcats.emplace( std::piecewise_construct,
                                      std::forward_as_tuple(arch),
                                      std::forward_as_tuple(*simple_compare) );
        cats_to_strict_supercats.emplace( std::piecewise_construct,
                                        std::forward_as_tuple(arch),
                                        std::forward_as_tuple(*simple_compare) );
        cats_to_strict_subcats.emplace( std::piecewise_construct,
                                      std::forward_as_tuple(arch),
                                      std::forward_as_tuple(*simple_compare) );
        for( TreePtr<Node> candidate : categories )
        {
            bool weak_super = candidate->IsLocalMatch( arch.get() );
            bool weak_sub = arch->IsLocalMatch( candidate.get() );
            if( weak_super )
                cats_to_nonstrict_supercats.at(arch).insert(candidate);
            if( weak_sub )
                cats_to_nonstrict_subcats.at(arch).insert(candidate);
            if( weak_super && !weak_sub )
                cats_to_strict_supercats.at(arch).insert(candidate);
            if( weak_sub && !weak_super )
                cats_to_strict_subcats.at(arch).insert(candidate);
        }
    }

    // Put architypes into a vector.
    for( TreePtr<Node> arch : categories )
    {
        cats_in_lacing_order.push_back( arch );
    }
    
    // Determine lacing ordering, non-cyclic, weighted by number of 
    // strict subcategories. We will apply swaps of contiguous blocks of 
    // architypes for as long as we can reduce total distance metric on lace.
    int n = cats_in_lacing_order.size();
    int count_outer = 0;
    bool swapped;
    do
    {
        swapped = false;
        for( int bs=n/2; bs>=1; bs-- ) // Block sizes
        {
            for( int i=0; i+bs<=n; i++ )
            {
                for( int j=0; j+bs<=i; j++ )
                {
                    ASSERT( i-j >= bs );
                    int curr_metric = 0, metric_if_swapped = 0;
                    
                    // If blocks touch, swaps will change neigbours and
                    // a different formula is required. 
                    if( i-j != n-bs ) 
                    {
                        curr_metric += GetMetric(i+bs-1, i+bs) + GetMetric(j, j-1);
                        metric_if_swapped += GetMetric(i, j-1) + GetMetric(j+bs-1, i+bs);
                    }
                    else
                    {
                        curr_metric += 0;
                        metric_if_swapped += 0;
                    }
                    
                    if( i-j != bs ) 
                    {
                        curr_metric += GetMetric(i, i-1) + GetMetric(j+bs-1, j+bs);
                        metric_if_swapped += GetMetric(i+bs-1, j+bs) + GetMetric(j, i-1);
                    }
                    else
                    {
                        curr_metric += GetMetric(j+bs-1, i);
                        metric_if_swapped += GetMetric(i+bs-1, j);
                    }
                    
                    if( metric_if_swapped < curr_metric )
                    {
                        int expected_reduction = curr_metric - metric_if_swapped;
                        int tot=0;
                        vector<int> old_metrics;
                        for( int k=0; k<n; k++ )               
                        {         
                            old_metrics.push_back(GetMetric(k, k+1));                        
                            tot += GetMetric(k, k+1);
                        }
                        
                        for( int k=0; k<bs; k++ )
                            cats_in_lacing_order.at((i+k)%n).swap( cats_in_lacing_order.at((j+k)%n) );
                        
                        for( int k=0; k<n; k++ )                        
                            tot -= GetMetric(k, k+1);                        
                        TRACEC("Swap i=%d j=%d bs=%d expecting %d (%d-%d) got %d\n", i, j, bs, expected_reduction, curr_metric, metric_if_swapped, tot);
                        if( expected_reduction != tot )
                        {
                            for( int k=0; k<n; k++ )
                                TRACEC("%d: %d -> %d\n", k, old_metrics[k], GetMetric(k, k+1) );
                        }
                        ASSERT( expected_reduction==tot );
                        swapped = true;
                    }
                }
            }
        }
        count_outer++;        
    } while( swapped );    
    TRACE("count_outer=%d\n", count_outer);
    
    // Shows ordering and supercats together 
    if( Tracer::IsEnabled() )
    {
        vector< tuple<int, TreePtr<Node>, set<TreePtr<Node>, SimpleCompare &> >> for_trace;
        for( int i=0; i<n; i++ )
        {
            auto arch = cats_in_lacing_order.at(i);
            for_trace.push_back( make_tuple( GetMetric(i, i+1), arch, cats_to_strict_supercats.at(arch) ) );
        }
        TRACE(for_trace)("\n");
    }
}


int TheKnowledge::Plan::GetMetric(int p, int q)
{
    int n = cats_in_lacing_order.size();
    // Non-cyclic, so if we go off the end the score is zero
    if( p<0 || p>=n || q<0 || q>=n )
        return 0;
    ASSERT( p != q ); // p is candidate to swap and q is neighbour. Swapping p must not change q.
    const TreePtr<Node> &pa = cats_in_lacing_order.at(p);
    const TreePtr<Node> &qa = cats_in_lacing_order.at(q);
    const set<TreePtr<Node>, SimpleCompare&> &psupers = cats_to_nonstrict_supercats.at(pa);
    const set<TreePtr<Node>, SimpleCompare&> &qsupers = cats_to_nonstrict_supercats.at(qa);
    set<TreePtr<Node>, SimpleCompare&> diff = SymmetricDifferenceOf( psupers, qsupers );
    int metric = 0;
    for( TreePtr<Node> t : diff )
        metric += cats_to_strict_subcats.at(t).size(); // weight for bigness of the categories in the diff
    //TRACEC("p=%d q=%d: ", p, q)(pa)(" ")(qa)(" metric=%d\n", metric);
    return metric;
}


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


void TheKnowledge::Update( PatternLink root_plink, XLink root_xlink )
{
    Clear();
    Build( root_plink, root_xlink );
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


