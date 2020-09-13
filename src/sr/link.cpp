#include "link.hpp"
#include "agent.hpp"

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


PatternLink::PatternLink(const TreePtrInterface *ppattern_, void *whodat_) :
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

PatternLink::operator bool() const
{
    return ppattern != nullptr;
}


Agent *PatternLink::GetChildAgent() const
{
    ASSERT( ppattern )
          ("GetChildAgent() called on uninitialised (NULL) link\n");
    return Agent::AsAgent(*ppattern);    
}


LocatedLink::LocatedLink() :
    ppattern( nullptr )
{
#ifdef KEEP_WHODAT_INFO
    whodat = nullptr;
#endif
}


LocatedLink::LocatedLink( const TreePtrInterface *ppattern_, 
                          const TreePtr<Node> &x_,
                          void *whodat_ ) :
    ppattern( ppattern_ ),
    x( x_ )
{
    EnsureNotOnStack( ppattern );
    ASSERT(x);

#ifdef KEEP_WHODAT_INFO
    whodat = whodat_;
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


LocatedLink::operator bool() const
{
    return ppattern != nullptr;
}


Agent *LocatedLink::GetChildAgent() const
{
    ASSERT( ppattern )
          ("GetChildAgent() called on uninitialised (NULL) link\n");
    return Agent::AsAgent(*ppattern);
}


const TreePtr<Node> &LocatedLink::GetChildX() const
{
    return x;
}


LocatedLink::operator PatternLink() const
{
    return PatternLink( ppattern );
}
