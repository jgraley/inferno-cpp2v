#include "link.hpp"
#include "agents/agent.hpp"

using namespace SR;

#define XLINK_BY_ARROWHEAD

// For debugging
#ifdef KEEP_WHODAT_INFO
#define WHODAT() __builtin_extract_return_addr (__builtin_return_address (0))
#else
#define WHODAT() nullptr
#endif
 
void EnsureNotOnStack( const TreePtrInterface *p )
{
    ASSERT( (((uint64_t)p & 0x7fff00000000ULL) != 0x7fff00000000ULL) )
          ("Supplied link ")(p)(" seems like it's probably on the stack, usually a bad sign\n");
}

//////////////////////////// PatternLink ///////////////////////////////

PatternLink::PatternLink()
{
#ifdef KEEP_WHODAT_INFO
    whodat = WHODAT();
#endif
}


PatternLink::PatternLink(shared_ptr<const Node> parent_pattern,
                         const TreePtrInterface *ppattern, 
                         void *whodat_) :
    asp_pattern( parent_pattern, ppattern )
{
    EnsureNotOnStack( ppattern );
#ifdef KEEP_WHODAT_INFO
    whodat = whodat_ ? whodat_ : WHODAT();
#endif    
}


PatternLink::PatternLink(const Agent *parent_agent,
                         const TreePtrInterface *ppattern) :
    PatternLink( parent_agent->GetPatternPtr(), ppattern, WHODAT() )
{
}


PatternLink PatternLink::CreateDistinct( const TreePtr<Node> &tp_pattern )
{
    shared_ptr< TreePtr<Node> > sp_tp_pattern = make_shared< TreePtr<Node> >( tp_pattern ); 
    return PatternLink(sp_tp_pattern, WHODAT());
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


PatternLink::PatternLink(shared_ptr<const TreePtrInterface> ppattern, 
                         void *whodat_) :
    asp_pattern( ppattern )
{
#ifdef KEEP_WHODAT_INFO
    whodat = whodat_ ? whodat_ : WHODAT();
#endif    
}

//////////////////////////// XLink ///////////////////////////////

XLink::XLink() :
    asp_x( nullptr )
{
#ifdef KEEP_WHODAT_INFO
    whodat = WHODAT();
#endif
}


XLink::XLink( shared_ptr<const Node> parent_x,
              const TreePtrInterface *px,
              void *whodat_ ) :
    asp_x( parent_x, px )              
{
    ASSERT(asp_x);
    ASSERT(*asp_x);

#ifdef KEEP_WHODAT_INFO
    whodat = whodat_ ? whodat_ : WHODAT();
#endif  
}


XLink::XLink( const LocatedLink &l ) :
    XLink( l.xlink )              
{
}


XLink XLink::FromWalkIterator( const Walk::iterator &wit, XLink root )
{
    TreePtr<Node> parent_x = wit.GetCurrentParent();
    const TreePtrInterface *px = wit.GetCurrentParentPointer();
    ASSERT( root || (parent_x && px) );
    return parent_x ? XLink( parent_x, px ) : root;    
}

              
XLink XLink::CreateDistinct( const TreePtr<Node> &tp_x )
{
    shared_ptr< TreePtr<Node> > sp_tp_x = make_shared< TreePtr<Node> >( tp_x ); 
    return XLink(sp_tp_x, WHODAT());
}
              
              
bool XLink::operator<(const XLink &other) const
{
#ifdef XLINK_BY_ARROWHEAD
    return asp_x < other.asp_x;    
#else
    return *asp_x < *other.asp_x;    
#endif    
}

    
bool XLink::operator!=(const XLink &other) const
{
#ifdef XLINK_BY_ARROWHEAD
    return asp_x != other.asp_x;    
#else
    return *asp_x != *other.asp_x;    
#endif
}


bool XLink::operator==(const XLink &other) const
{
#ifdef XLINK_BY_ARROWHEAD
    return asp_x == other.asp_x;    
#else
    return *asp_x == *other.asp_x;    
#endif
}


XLink::operator bool() const
{
    return asp_x != nullptr;
}


TreePtr<Node> XLink::GetChildX() const
{
    return *asp_x;
}


string XLink::GetTrace() const // used for debug
{
    return string("->") + asp_x->GetTrace();
}


XLink::XLink( shared_ptr<const TreePtrInterface> px,
              void *whodat_ ) :
    asp_x( px )
{
    ASSERT(asp_x);
    ASSERT(*asp_x);

#ifdef KEEP_WHODAT_INFO
    whodat = whodat_ ? whodat_ : WHODAT();
#endif  
}              


const XLink XLink::MMAX_Link = XLink::CreateDistinct( MakeTreePtr<XLink::MMAX>() );
              
//////////////////////////// LocatedLink ///////////////////////////////

LocatedLink::LocatedLink()  
{
}


LocatedLink::LocatedLink( const PatternLink &plink_, 
                          const XLink &xlink_) :
    plink( plink_ ),
    xlink( xlink_ )                                                  
{
    ASSERT( (bool)plink == (bool)xlink );
}


bool LocatedLink::operator<(const LocatedLink &other) const
{
    // pattern is primary ordering for consistency with 
    // PatternLink
    if( plink != other.plink )
        return plink < other.plink;
        
    return xlink < other.xlink;    
}


bool LocatedLink::operator!=(const LocatedLink &other) const
{
    return plink != other.plink || xlink != other.xlink;
}


bool LocatedLink::operator==(const LocatedLink &other) const
{
    return plink == other.plink && xlink == other.xlink;
}


LocatedLink::operator bool() const
{
    ASSERT( (bool)plink == (bool)xlink );
    return (bool)plink;
}


LocatedLink::operator pair<const PatternLink, XLink>() const
{
    return make_pair( plink, xlink );
}


Agent *LocatedLink::GetChildAgent() const
{
    return plink.GetChildAgent();
}


const TreePtrInterface *LocatedLink::GetPatternPtr() const
{
    return plink.GetPatternPtr();
}


TreePtr<Node> LocatedLink::GetChildX() const
{
    return xlink.GetChildX();
}


LocatedLink::operator PatternLink() const
{
    return plink;
}


string LocatedLink::GetTrace() const
{
    return plink.GetTrace() + string(":=") + xlink.GetTrace();      
}

//////////////////////////// free functions ///////////////////////////////

/*
 * Not too sure about the use of GetChildAgent() in content compare. Also
 * has no callers.
 
bool SR::operator==( const list<PatternLink> &left, const list<LocatedLink> &right )
{
    ASSERT(false);
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
*/

list<LocatedLink> SR::LocateLinksFromMap( const list<PatternLink> &plinks, 
                                          const map< PatternLink, XLink > &basic_solution )
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
