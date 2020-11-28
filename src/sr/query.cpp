#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

#include <inttypes.h>

using namespace SR;

PatternQuery::PatternQuery( const Agent *base_agent_ ) :
    base_agent( base_agent_ )
{
}
 
 
void PatternQuery::clear()
{
    normal_links.clear();
    abnormal_links.clear();
    multiplicity_links.clear();
    evaluator = shared_ptr<BooleanEvaluator>();
}


void PatternQuery::RegisterDecision( bool inclusive )
{
    Decision d;
    d.inclusive = inclusive;
    decisions.push_back(d);
}

 
void PatternQuery::RegisterNormalLink( PatternLink plink )
{
    normal_links.push_back( plink );        
}


void PatternQuery::RegisterAbnormalLink( PatternLink plink )
{
    abnormal_links.push_back( plink );       
}


void PatternQuery::RegisterMultiplicityLink( PatternLink plink )
{
    multiplicity_links.push_back( plink );       
}


PatternQuery::Links PatternQuery::GetAllLinks() const
{
    Links links;
    for( auto link : GetNormalLinks() )
        links.push_back( link );
    for( auto link : GetAbnormalLinks() )
        links.push_back( link );
    for( auto link : GetMultiplicityLinks() )
        links.push_back( link );
    return links;
}


void DecidedQueryCommon::CheckMatchingLinks( const DecidedQueryCommon::Links &mut_links, 
                                             const DecidedQueryCommon::Links &ref_links )
{    
    TRACE("Checking ")(mut_links)(" against ")(ref_links)("\n");
    
    // Multiplicity X links are not uniquified but their contents should match 
    list<LocatedLink>::const_iterator mit = mut_links.begin();
    list<LocatedLink>::const_iterator rit = ref_links.begin();
    while( mit != mut_links.end() || rit != ref_links.end() )
    {
        ASSERT( mit != mut_links.end() && rit != ref_links.end() );
        ASSERT( (PatternLink)*mit == (PatternLink)*rit );
        auto mxp = mit->GetChildX().get();
        auto rxp = rit->GetChildX().get();
        
        if( auto mxssr = dynamic_cast<SubSequenceRange *>(mxp) )
        {
            auto rxssr = dynamic_cast<SubSequenceRange *>(rxp);
            ASSERT( rxssr );
            ASSERT( mxssr->begin() == rxssr->begin() );
            ASSERT( mxssr->end() == rxssr->end() );
        }
        else if( auto mxscl = dynamic_cast<SubCollection *>(mxp) )
        {
            auto rxscl = dynamic_cast<SubCollection *>(rxp);
            ASSERT( rxscl );
            ASSERT( mxscl->elts == rxscl->elts )
                  (mxscl->elts)(" != ")(rxscl->elts);
         }
        else if( auto mxssl = dynamic_cast<SubSequence *>(mxp) )
        {
            auto rxssl = dynamic_cast<SubSequence *>(rxp);
            ASSERT( rxssl );
            ASSERT( mxssl->elts == rxssl->elts )
                  (mxssl->elts)(" != ")(rxssl->elts);
        }    
        else // some other node: should match by link
        {
            ASSERT( *mit == *rit );
        }
        ++mit;
        ++rit;
    }
}


string DecidedQueryCommon::TraceLinks( const DecidedQueryCommon::Links &links )
{      
    string s = "[";
    for( LocatedLink link : links )
    {
        auto xsc = dynamic_cast<SubContainer *>( link.GetChildX().get() );
        
        s += Trace((PatternLink)link) + ":=";
        if( auto xscr = dynamic_cast<SubContainerRange *>(xsc) )
        {
            ASSERT( link );
            ContainerInterface *xci = dynamic_cast<ContainerInterface *>(xscr);
            ASSERT(xci)("Multiplicity x must implement ContainerInterface");    
            
            s += "SubContainerRange[";
            FOREACH( const TreePtrInterface &xe_node, *xci )
            {
                XLink xe_link = XLink(xscr->GetParentX(), &xe_node);
                s += Trace(xe_link) + ", ";
            }
            s += "],\n";
        }
        else if( auto xscl = dynamic_cast<SubCollection *>(xsc) )
        {
            s += "SubCollection[";
            for( XLink xe_link : xscl->elts )
                s += Trace(xe_link) + ", ";
            s += "],\n";
        }
        else if( auto xssl = dynamic_cast<SubSequence *>(xsc) )
        {
            s += "SubSequence[";
            for( XLink xe_link : xssl->elts )
                s += Trace(xe_link) + ", ";
            s += "],\n";
        }    
        else
        {
            s += Trace((XLink)link) + ",\n";
        }
    }
    return s + "]";
}


string DecidedQuery::GetTrace() const
{
    string s;
    s += "Normal: " + TraceLinks(GetNormalLinks()) + "\n";
    s += "Abormal: " + TraceLinks(GetAbnormalLinks()) + "\n";
    s += "Multiplicity: " + TraceLinks(GetMultiplicityLinks()) + "\n";

    ASSERT( choices.size() == decisions.size() );

    s += "Decisions: [";
    for( int i=0; i<decisions.size(); i++ )
    {
        Range d = decisions[i];
        Choice c = choices[i];
        s += SSPrintf("%d:", i);
        s += Trace(d) + ", ";
        s += "choice=";
        if( c.mode == Choice::BEGIN )
            s += "BEGIN";
        else if( d.container )
            s += (c.iter==d.container->end() ? "END" : Trace(*(c.iter)));
        else
            s += Trace(*(c.iter));
        s += ",\n";
    }
    s += "]";
    return s;
}


DecidedQuery::DecidedQuery( shared_ptr<const PatternQuery> pq ) :
    base_agent( pq->GetBaseAgent() ),
    decisions( pq->GetDecisions().size() ),
    next_decision( decisions.begin() ), 
    choices( pq->GetDecisions().size() ),
    next_choice( choices.begin() ) 
{
    CompleteDecisionsWithEmpty();
}


void DecidedQuery::Start()
{
    for( Range &r : decisions )
        r = empty_range;
}


void DecidedQuery::RegisterNormalLink( PatternLink plink, XLink xlink )
{
    LocatedLink link( plink, xlink );
    normal_links.push_back( link );        
}


void DecidedQuery::RegisterAbnormalLink( PatternLink plink, XLink xlink )
{
    LocatedLink link( plink, xlink );
    abnormal_links.push_back( link );
}


void DecidedQuery::RegisterMultiplicityLink( PatternLink plink, XLink xlink )
{
    LocatedLink link( plink, xlink );
    multiplicity_links.push_back( link );
}


void PatternQuery::RegisterEvaluator( shared_ptr<BooleanEvaluator> e )
{
	ASSERT( !evaluator ); // should not register more than one
	evaluator = e;
}	


void DecidedQuery::RegisterEvaluator( shared_ptr<BooleanEvaluator> e )
{
	ASSERT( !evaluator ); // should not register more than one
	evaluator = e;
}	
        
        
void DecidedQuery::Invalidate( int bc )
{
    // TODO may not need all thes preconditions
    ASSERT( !choices.empty() );
    ASSERT( bc >= 0 );
    ASSERT( bc < choices.size() );
    if( next_decision > decisions.begin() + bc ) 
        next_decision = decisions.begin() + bc; 
  
    //decisions.pop_back();    
    choices[bc].mode = Choice::BEGIN;    
}


void DecidedQuery::SetChoice( int bc, Choice newc )
{
    ASSERT( !choices.empty() );
    ASSERT( bc >= 0 );
    ASSERT( bc < choices.size() );
    choices[bc] = newc;
}


void DecidedQuery::PushBackChoice( Choice newc )
{
    choices.push_back(newc);
}


void DecidedQuery::EnsureChoicesHaveIterators()
{
    for( int i=0; i<choices.size(); i++ )
    {
        DecidedQueryCommon::Choice &choice = choices[i];
        if( choice.mode == DecidedQueryCommon::Choice::BEGIN )
        {        
            choice.mode = DecidedQueryCommon::Choice::ITER;
            choice.iter = decisions[i].begin;
        }
    }
}


ContainerInterface::iterator DecidedQuery::RegisterDecision( const Range &r )
{
    ASSERT( r.inclusive || r.begin != r.end )("no empty decisions"); 
    ContainerInterface::iterator it;
    ASSERT( next_decision != decisions.end() ); // run out of decisions? Shouldn't happen now.
    ASSERT( next_choice != choices.end() );
    switch( next_choice->mode )
    {
        case Choice::ITER:
            // We have an iterator already: any decision submitted here must match the previous one
            // so that the iterator remains valid. See range::operator==(). If this cannot be achieved, 
            // then the agent must use IsNextChoiceValid() and if it returns true, call SkipDecision()
            it = next_choice->iter; // Use the iterator that was given to us
            ASSERT( r == *next_decision );
            break;
        
        case Choice::BEGIN:
            // We do not have an iterator: we are able to change the decision
            it = r.begin; // we have been asked to use begin
            *next_decision = r;
            break;
    }
    ASSERT( r.inclusive || it != r.end )("no empty decisions"); 
    ASSERT( it == r.end || *it )("A choice cannot be a nullptr");
    ++next_decision; 
    ++next_choice;
    
    return it;
}                                                      


ContainerInterface::iterator DecidedQuery::RegisterDecision( ContainerInterface::iterator begin,
                                                             ContainerInterface::iterator end,
                                                             bool inclusive,
                                                             shared_ptr<ContainerInterface> container )
{
    Range r( begin, end, inclusive, container);
    return RegisterDecision( r );
}
                                                                    
                               
ContainerInterface::iterator DecidedQuery::RegisterDecision( shared_ptr<ContainerInterface> container, bool inclusive )
{
    ASSERT( container );
    return RegisterDecision( container->begin(), container->end(), inclusive, container );
}                                                      


ContainerInterface::iterator DecidedQuery::RegisterDecision( const Sequence<Node> &container, bool inclusive )
{
    auto container_for_query = make_shared< Sequence<Node> >();
    *container_for_query = container;
    ContainerInterface::iterator query_it = RegisterDecision( container_for_query, inclusive );
    //auto p_myit = dynamic_cast<const Sequence<Node>::iterator *>(query_it.GetUnderlyingIterator()); // for #109
    return query_it;
}                                                      


ContainerInterface::iterator DecidedQuery::RegisterDecision( const Collection<Node> &container, bool inclusive )
{
    auto container_for_query = make_shared< Collection<Node> >();
    *container_for_query = container;
    ContainerInterface::iterator query_it = RegisterDecision( container_for_query, inclusive );
    //auto p_myit = dynamic_cast<const Collection<Node>::iterator *>(query_it.GetUnderlyingIterator()); // for #109
    return query_it;
}                                                      


bool DecidedQuery::IsNextChoiceValid() const
{
    return next_choice->mode == Choice::ITER;
}


const DecidedQueryCommon::Range &DecidedQuery::GetNextOldDecision() const
{
    ASSERT( next_decision != decisions.end() )
          ("%d [%p %p ... %p] %p\n", decisions.size(), &decisions[0], &decisions[1], &decisions.back(), &*next_decision);
    return *next_decision;
}


ContainerInterface::iterator DecidedQuery::SkipDecision()
{
    ContainerInterface::iterator it;
    ASSERT( next_decision != decisions.end() ); // run out of decisions? Shouldn't happen now.
    ASSERT( next_choice != choices.end() );
    switch( next_choice->mode )
    {
        case Choice::ITER:
            it = next_choice->iter; // Use the iterator that was given to us
            break;
        
        case Choice::BEGIN:
            it = next_decision->begin; // we have been asked to use begin
            break;
    }
    ASSERT( it == next_decision->end || *it )("A choice cannot be a nullptr");
    ++next_decision; 
    ++next_choice;
    
    return it;
}


void DecidedQuery::CompleteDecisionsWithEmpty()
{
    // This part needed after each DQ
    auto nd = next_decision;
    while( nd != decisions.end() )
    {
        *nd = empty_range;
        ++nd; 
    }    
}


void DecidedQuery::Reset()
{
    normal_links.clear();
	abnormal_links.clear();
    multiplicity_links.clear();
    evaluator = shared_ptr<BooleanEvaluator>();    
    next_decision = decisions.begin();  
    next_choice = choices.begin();  
}


DecidedQuery::Links DecidedQuery::GetAllLinks() const
{
    Links links;
    for( auto link : GetNormalLinks() )
        links.push_back( link );
    for( auto link : GetAbnormalLinks() )
        links.push_back( link );
    for( auto link : GetMultiplicityLinks() )
        links.push_back( link );
    return links;
}


DecidedQueryCommon::Range::Range( ContainerInterface::iterator begin_,
                                  ContainerInterface::iterator end_,    
                                  bool inclusive_,
                                  std::shared_ptr<ContainerInterface> container_ ) :
    begin(begin_), end(end_), inclusive(inclusive_), container(container_)
{
}


bool DecidedQueryCommon::Range::operator==(const Range &o) const // Only required for an ASSERT
{
    if( begin != o.begin )
        return false;
    if( end != o.end )
        return false;
    if( inclusive != o.inclusive )
        return false;
    if( container != o.container )
        return false;
    return true;
}


string DecidedQueryCommon::Range::GetTrace() const 
{
    string s;
    s += "(";
    if( container )
        s += SSPrintf("container=%p", container.get()) + ", ";
    else
        s += SSPrintf("container=%p", container.get()) + ", ";
            
    if( container )
        s += "begin=@" + (begin==container->end() ? string("END") : Trace(*begin)) + ", ";
    
    if( container )
        s += "end=@" + (end==container->end() ? string("END") : Trace(*end)) + ", ";
    
    s += "inclusive=" + Trace(inclusive) + ", ";
    
    s += "range=[";
    for( ContainerInterface::iterator it = begin;
         it != end;
         ++it )
        s += Trace(*it) + ", ";
    s += "] ";

    s += ")";
    return s;
}


shared_ptr< Collection<Node> > DecidedQuery::empty_container = make_shared< Collection<Node> >();
DecidedQuery::Range DecidedQuery::empty_range { empty_container->begin(), empty_container->end(), false, empty_container };
