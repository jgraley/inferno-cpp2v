#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

#include <inttypes.h>

using namespace SR;

//////////////////////////// PatternQuery ///////////////////////////////

PatternQuery::PatternQuery()
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

//////////////////////////// DecidedQueryCommon ///////////////////////////////

void DecidedQueryCommon::AssertMatchingLinks( const DecidedQueryCommon::Links &mut_links, 
                                              const DecidedQueryCommon::Links &ref_links )
{    
    TRACE("Checking ")(mut_links)(" against ")(ref_links)("\n");
    
    // Multiplicity X links are not uniquified but their contents should match 
    ASSERT( mut_links.size() == ref_links.size() );
    for( pair<const PatternLink, XLink> rp : ref_links )
    {
        PatternLink plink = rp.first;
        XLink ref_xlink = rp.second;
        ASSERT( mut_links.count(plink) == 1 );
        XLink mut_xlink = mut_links.at(plink);        
        if( auto mxssr = dynamic_cast<SubContainer *>(mut_xlink.GetChildTreePtr().get()) )        
            mxssr->AssertMatchingContents( ref_xlink.GetChildTreePtr() ); // only the contents will actually match        
        else // some other node: should match by link        
            ASSERT( mut_xlink == ref_xlink );        
    }
}


string DecidedQueryCommon::TraceLinks( const DecidedQueryCommon::Links &links )
{      
    bool first = true;
    string s = "Links[";
    for( LocatedLink link : links )
    {
        if( !first )
            s += ",\n";
        first = false;                
        s += Trace((PatternLink)link) + ":=";
        if( auto xsc = dynamic_cast<SubContainer *>( link.GetChildTreePtr().get() ) )
            s += xsc->GetContentsTrace(); // normal trace is not enough info
        else
            s += Trace((XLink)link);
    }
    return s + "]";
}

//////////////////////////// DecidedQuery ///////////////////////////////

string DecidedQuery::GetTrace() const
{
    string s;
    s += "Normal: " + TraceLinks(GetNormalLinks()) + "\n";
    s += "Abnormal: " + TraceLinks(GetAbnormalLinks()) + "\n";
    s += "Multiplicity: " + TraceLinks(GetMultiplicityLinks()) + "\n";

    ASSERT( choices.size() == decisions.size() );

    bool first = true;
    s += "Decisions: [";
    for( Ranges::size_type i=0; i<decisions.size(); i++ )
    {
        if( !first )
            s += ",\n";
        first = false;
        Range d = decisions[i];
        Choice c = choices[i];
        s += SSPrintf("(%d:", i);
        s += Trace(d) + ", ";
        s += "choice=" + c.GetTrace(d) + ")";
    }
    s += "]";
    return s;
}


DecidedQuery::DecidedQuery( shared_ptr<const PatternQuery> pq ) :
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
    normal_links.insert( link );        
}


void DecidedQuery::RegisterAbnormalLink( PatternLink plink, XLink xlink )
{
    LocatedLink link( plink, xlink );
    abnormal_links.insert( link );
}


void DecidedQuery::RegisterMultiplicityLink( PatternLink plink, XLink xlink )
{
    LocatedLink link( plink, xlink );
    multiplicity_links.insert( link );
}


void PatternQuery::RegisterEvaluator( shared_ptr<BooleanEvaluator> e )
{
    ASSERT( !evaluator ); // should not register more than one
    evaluator = e;
}    


void DecidedQuery::Invalidate( Choices::size_type bc )
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


void DecidedQuery::SetChoice( Choices::size_type bc, Choice newc )
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
            // so that the iterator remains valid. See range::operator==().
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
    next_decision = decisions.begin();  
    next_choice = choices.begin();  
}


DecidedQuery::Links DecidedQuery::GetAllLinks() const
{
    Links links = GetNormalLinks();
    links = UnionOfSolo( links, GetAbnormalLinks() );
    links = UnionOfSolo( links, GetMultiplicityLinks() );
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
    // Inconvenient: it suffices that we supply a new container 
    // with the same begin, end, inclusive and iteration behaviour
    //if( container != o.container )
    //    return false; 
    return true;
}


string DecidedQueryCommon::Range::GetTrace() const 
{
    list<string> vs;
    if( container )
    {
        vs.push_back( SSPrintf("container-size=%d", container->size()) );    
        vs.push_back( "begin=@" + (begin==container->end() ? string("END") : Trace(*begin)) );
        vs.push_back( "end=@" + (end==container->end() ? string("END") : Trace(*end)) );
    }
        
    vs.push_back( "inclusive=" + Trace(inclusive) );
    
    list<string> vs2;
    for( ContainerInterface::iterator it = begin;
         it != end;
         ++it )
        vs2.push_back( Trace(*it) );
    
    vs.push_back( "which is "+Join(vs2, ", ", "[", "]" ) );

    return "Range" + Join(vs, ", ", "(", ")" );

}


string DecidedQueryCommon::Choice::GetTrace( const Range &d ) const
{
    string s;
    if( mode == Choice::BEGIN )
        s += "BEGIN";
    else if( iter==d.end )
        s += "END";
    else if( d.container && iter==d.container->end() )
        s += "END";
    else
        s += Trace(*iter);
    return s;
}


shared_ptr< Collection<Node> > DecidedQuery::empty_container = make_shared< Collection<Node> >();
DecidedQuery::Range DecidedQuery::empty_range { empty_container->begin(), empty_container->end(), false, empty_container };
