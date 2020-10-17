#include "link.hpp"
#include "agents/agent.hpp"

using namespace SR;


void EnsureNotOnStack( const TreePtrInterface *p )
{
    ASSERT( (((uint64_t)p & 0x7fff00000000ULL) != 0x7fff00000000ULL) )
          ("Supplied link ")(p)(" seems like it's probably on the stack, usually a bad sign\n");
}

//////////////////////////// PatternLink ///////////////////////////////

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

//////////////////////////// XLink ///////////////////////////////

XLink::XLink() :
    x( nullptr )
{
#ifdef KEEP_WHODAT_INFO
    whodat = nullptr;
#endif
}


XLink::XLink( const TreePtr<Node> &x_,
              void *whodat ) :
    x( x_ )              
{
    ASSERT(x);

#ifdef KEEP_WHODAT_INFO
    whodat = whodat_;
#endif  
}

              
XLink::XLink( const LocatedLink &l ) :
    x( l.x )              
{
    ASSERT(x);

#ifdef KEEP_WHODAT_INFO
    whodat = l.whodat;
#endif  
}

              
bool XLink::operator<(const XLink &other) const
{
    return x < other.x;    
}

    
bool XLink::operator!=(const XLink &other) const
{
    return x != other.x;    
}


bool XLink::operator==(const XLink &other) const
{
    return x == other.x;    
}


XLink::operator bool() const
{
    return x != nullptr;
}


const TreePtr<Node> &XLink::GetChildX() const
{
    return x;
}


string XLink::GetTrace() const // used for debug
{
    return string("@") + x->GetTrace();
}

//////////////////////////// LocatedLink ///////////////////////////////

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
#ifdef LINKS_ENHANCED_TRACE
    parent_pattern( parent_pattern_ ),
#endif
    ppattern( ppattern_ ),
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
#ifdef LINKS_ENHANCED_TRACE
    parent_pattern( plink.parent_pattern ),
#endif
    ppattern( plink.ppattern ),
    x( x_ )                                                  
{
    ASSERT(x);

#ifdef KEEP_WHODAT_INFO
    whodat = plink.whodat;
#endif
}


LocatedLink::LocatedLink( const Agent *parent_pattern_,
                          const TreePtrInterface *ppattern_, 
                          const XLink &xlink ) :
#ifdef LINKS_ENHANCED_TRACE
    parent_pattern( parent_pattern_ ),
#endif
    ppattern( ppattern_ ),
    x( xlink.x )
{
    EnsureNotOnStack( ppattern );
    ASSERT(x);

#ifdef KEEP_WHODAT_INFO
    whodat = whodat_;
#endif  
}


LocatedLink::LocatedLink( const PatternLink &plink, 
                          const XLink &xlink) :
#ifdef LINKS_ENHANCED_TRACE
    parent_pattern( plink.parent_pattern ),
#endif
    ppattern( plink.ppattern ),
    x( xlink.x )                                                  
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


LocatedLink::operator pair<const PatternLink, TreePtr<Node>>() const
{
    return make_pair( (PatternLink)*this, GetChildX() );
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
                                          const map< PatternLink, TreePtr<Node> > &my_solution )
{
    list<LocatedLink> llinks;
    for( PatternLink plink : plinks )
    {
        ASSERT( my_solution.count(plink) > 0 );
        LocatedLink llink( plink, my_solution.at(plink) );
        llinks.push_back( llink );
    }
    return llinks;
}                                      
