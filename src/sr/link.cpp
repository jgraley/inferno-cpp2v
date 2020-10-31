#include "link.hpp"
#include "agents/agent.hpp"

using namespace SR;


void EnsureNotOnStack( const TreePtrInterface *p )
{
    ASSERT( (((uint64_t)p & 0x7fff00000000ULL) != 0x7fff00000000ULL) )
          ("Supplied link ")(p)(" seems like it's probably on the stack, usually a bad sign\n");
}

//////////////////////////// PatternLink ///////////////////////////////

PatternLink::PatternLink()
{
#ifdef KEEP_WHODAT_INFO
    whodat = nullptr;
#endif
}


PatternLink::PatternLink(shared_ptr<const Node> parent_pattern,
                         const TreePtrInterface *ppattern, 
                         void *whodat_) :
    asp_pattern( parent_pattern, ppattern )
{
    EnsureNotOnStack( ppattern );
#ifdef KEEP_WHODAT_INFO
    whodat = whodat_;
#endif    
}


PatternLink::PatternLink(const Agent *parent_agent,
                         const TreePtrInterface *ppattern, 
                         void *whodat_) :
    PatternLink( parent_agent->GetPatternPtr(), ppattern, whodat_ )
{
}


PatternLink::PatternLink(shared_ptr<const TreePtrInterface> ppattern, 
                         void *whodat_) :
    asp_pattern( ppattern )
{
#ifdef KEEP_WHODAT_INFO
    whodat = whodat_;
#endif    
}


bool PatternLink::operator<(const PatternLink &other) const
{
    // PatternLink is unique across parent-child links in
    // the pattern. This operator will permit PatternLink to 
    // act as keys in maps.
    return asp_pattern < other.asp_pattern;
}


bool PatternLink::operator!=(const PatternLink &other) const
{
    return asp_pattern != other.asp_pattern;
}


bool PatternLink::operator==(const PatternLink &other) const
{
    return asp_pattern == other.asp_pattern;
}


bool PatternLink::operator==(const LocatedLink &other) const
{
    return *this == other.plink;
}


PatternLink::operator bool() const
{
    return asp_pattern != nullptr;
}


Agent *PatternLink::GetChildAgent() const
{
    ASSERT( asp_pattern )
          ("GetChildAgent() called on uninitialised (nullptr) link\n");
    return Agent::AsAgent(*asp_pattern);    
}


TreePtr<Node> PatternLink::GetPattern() const
{
    return *asp_pattern;
}


const TreePtrInterface *PatternLink::GetPatternPtr() const
{
    return asp_pattern.get();
}


string PatternLink::GetTrace() const
{
    return string("->") + GetChildAgent()->GetTrace();
}

//////////////////////////// XLink ///////////////////////////////

XLink::XLink() :
    x( nullptr )
{
#ifdef KEEP_WHODAT_INFO
    whodat = nullptr;
#endif
}


XLink::XLink( const TreePtrInterface *px,
              void *whodat ) :
    x( *px )              
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

LocatedLink::LocatedLink()  
{
#ifdef KEEP_WHODAT_INFO
    whodat = nullptr;
#endif
}


LocatedLink::LocatedLink( shared_ptr<const Node> parent_pattern_,
                          const TreePtrInterface *ppattern_, 
                          const TreePtr<Node> &x_,
                          void *whodat_ ) :
    plink( parent_pattern_, ppattern_, whodat_ ),
    x( x_ )
{
    ASSERT(x);
}


LocatedLink::LocatedLink( const PatternLink &plink_, 
                          const TreePtr<Node> &x_ ) :
    plink( plink_ ),
    x( x_ )                                                  
{
    ASSERT(x);

#ifdef KEEP_WHODAT_INFO
    whodat = plink.whodat;
#endif
}


LocatedLink::LocatedLink( shared_ptr<const Node> parent_pattern_,
                          const TreePtrInterface *ppattern_, 
                          const XLink &xlink ) :
    plink( parent_pattern_, ppattern_ ),
    x( xlink.x )
{
    ASSERT(x);
}


LocatedLink::LocatedLink( const PatternLink &plink_, 
                          const XLink &xlink) :
    plink( plink_ ),
    x( xlink.x )                                                  
{
    ASSERT(x);
}


bool LocatedLink::operator<(const LocatedLink &other) const
{
    // pattern is primary ordering for consistency with 
    // PatternLink
    if( plink != other.plink )
        return plink < other.plink;
        
    return x < other.x;    
}


bool LocatedLink::operator!=(const LocatedLink &other) const
{
    return plink != other.plink || x != other.x;
}


bool LocatedLink::operator==(const LocatedLink &other) const
{
    return plink == other.plink && x == other.x;
}


LocatedLink::operator bool() const
{
    return (bool)plink;
}


LocatedLink::operator pair<const PatternLink, TreePtr<Node>>() const
{
    return make_pair( plink, x );
}


Agent *LocatedLink::GetChildAgent() const
{
    return plink.GetChildAgent();
}


const TreePtrInterface *LocatedLink::GetPatternPtr() const
{
    return plink.GetPatternPtr();
}


const TreePtr<Node> &LocatedLink::GetChildX() const
{
    return x;
}


LocatedLink::operator PatternLink() const
{
    return plink;
}


string LocatedLink::GetTrace() const
{
    return plink.GetTrace() + string("@") + x->GetTrace();      
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
                                          const map< PatternLink, TreePtr<Node> > &basic_solution )
{
    list<LocatedLink> llinks;
    for( PatternLink plink : plinks )
    {
        ASSERT( basic_solution.count(plink) > 0 );
        LocatedLink llink( plink, basic_solution.at(plink) );
        llinks.push_back( llink );
    }
    return llinks;
}                                      
