#include "link.hpp"
#include "agents/agent.hpp"

using namespace SR;


void EnsureNotOnStack( const TreePtrInterface *ppattern )
{
    ASSERT( (((uint64_t)ppattern & 0x7fff00000000ULL) != 0x7fff00000000ULL) )
          ("Supplied agent link seems like it's probably on the stack, usually a bad sign\n");
}


PatternLink::PatternLink() :
    ppattern( nullptr )
{
#ifdef KEEP_WHODAT_INFO
    whodat = nullptr;
#endif
}


PatternLink::PatternLink(const Agent *parent_pattern_,
                         const TreePtrInterface *ppattern_, 
                         void *whodat_) :
#ifdef LINKS_ENHANCED_TRACE
    parent_pattern( parent_pattern_ ),
#endif
    ppattern( ppattern_ )
{
    EnsureNotOnStack( ppattern );
#ifdef KEEP_WHODAT_INFO
    whodat = whodat_;
#endif    
}


bool PatternLink::operator<(const PatternLink &other) const
{
    // PatternLink is unique across parent-child links in
    // the pattern. This operator will permit PatternLink to 
    // act as keys in maps.
    return ppattern < other.ppattern;
}


bool PatternLink::operator!=(const PatternLink &other) const
{
    return ppattern != other.ppattern;
}


bool PatternLink::operator==(const PatternLink &other) const
{
    return ppattern == other.ppattern;
}


bool PatternLink::operator==(const LocatedLink &other) const
{
    return ppattern == other.ppattern;
}


PatternLink::operator bool() const
{
    return ppattern != nullptr;
}


Agent *PatternLink::GetChildAgent() const
{
    ASSERT( ppattern )
          ("GetChildAgent() called on uninitialised (nullptr) link\n");
    return Agent::AsAgent(*ppattern);    
}


TreePtr<Node> PatternLink::GetPattern() const
{
    return *ppattern;
}


const TreePtrInterface *PatternLink::GetPatternPtr() const
{
    return ppattern;
}


string PatternLink::GetTrace() const
{
#ifdef LINKS_ENHANCED_TRACE
    return parent_pattern->GetTrace() + "->" + GetChildAgent()->GetTrace();
#else
    return ".->" GetChildAgent()->GetTrace();
#endif    
}


LocatedLink::LocatedLink() :
    ppattern( nullptr )
{
#ifdef KEEP_WHODAT_INFO
    whodat = nullptr;
#endif
}


LocatedLink::LocatedLink( const Agent *parent_pattern_,
                          const TreePtrInterface *ppattern_, 
                          const TreePtr<Node> &x_,
                          void *whodat_ ) :
    parent_pattern( parent_pattern_ ),
#ifdef LINKS_ENHANCED_TRACE
    ppattern( ppattern_ ),
#endif
    x( x_ )
{
    EnsureNotOnStack( ppattern );
    ASSERT(x);

#ifdef KEEP_WHODAT_INFO
    whodat = whodat_;
#endif  
}


LocatedLink::LocatedLink( const PatternLink &plink, 
                          const TreePtr<Node> &x_ ) :
    parent_pattern( plink.parent_pattern ),
#ifdef LINKS_ENHANCED_TRACE
    ppattern( plink.ppattern ),
#endif
    x( x_ )                                                  
{
    ASSERT(x);

#ifdef KEEP_WHODAT_INFO
    whodat = plink.whodat;
#endif
}


bool LocatedLink::operator<(const LocatedLink &other) const
{
    // pattern is primary ordering for consistency with 
    // PatternLink
    if( ppattern != other.ppattern )
        return ppattern < other.ppattern;
        
    return x < other.x;    
}


bool LocatedLink::operator!=(const LocatedLink &other) const
{
    return ppattern != other.ppattern || x != other.x;
}


bool LocatedLink::operator==(const LocatedLink &other) const
{
    return ppattern == other.ppattern && x == other.x;
}


LocatedLink::operator bool() const
{
    return ppattern != nullptr;
}


Agent *LocatedLink::GetChildAgent() const
{
    ASSERT( ppattern )
          ("GetChildAgent() called on uninitialised (nullptr) link\n");
    return Agent::AsAgent(*ppattern);
}


const TreePtrInterface *LocatedLink::GetPatternPtr() const
{
    return ppattern;
}


const TreePtr<Node> &LocatedLink::GetChildX() const
{
    return x;
}


LocatedLink::operator PatternLink() const
{
#ifdef LINKS_ENHANCED_TRACE
    return PatternLink( parent_pattern, ppattern );
#else
    return PatternLink( nullptr, ppattern );
#endif    

}


string LocatedLink::GetTrace() const
{
#ifdef LINKS_ENHANCED_TRACE
    return parent_pattern->GetTrace() + "->" + GetChildAgent()->GetTrace() + string("@") + x->GetTrace();
#else
    return ".->" GetChildAgent()->GetTrace() + string("@") + x->GetTrace();
#endif       
}


bool SR::operator==( const list<PatternLink> &left, const list<LocatedLink> &right )
{
    auto left_it = left.begin();
    auto right_it = right.begin();
    while( left_it != left.end() || right_it != right.end() )         
    {
        if( left_it == left.end() || right_it == right.end() )
            return false; // differing length
        if( left_it->GetChildAgent() != right_it->GetChildAgent() )
            return false; // differing content
        ++left_it;
        ++right_it;
    }
    return true;
}


list<LocatedLink> SR::LocateLinksFromMap( const list<PatternLink> &plinks, 
                                          const map< PatternLink, TreePtr<Node> > &solution_keys,
                                          const map< Agent *, TreePtr<Node> > &mappy )
{
    list<LocatedLink> llinks;
    for( PatternLink plink : plinks )
    {
        if( solution_keys.count(plink) > 0 )
        {
            LocatedLink llink( plink, solution_keys.at(plink) );
            llinks.push_back( llink );
        }
        else
        {
            Agent *child_agent = plink.GetChildAgent();
            LocatedLink llink( plink, mappy.at(child_agent) );
            llinks.push_back( llink );
        }
    }
    return llinks;
}                                      
