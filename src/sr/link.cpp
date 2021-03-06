#include "link.hpp"
#include "agents/agent.hpp"

using namespace SR;

// For debugging
#ifdef KEEP_WHODAT_INFO
#define WHODAT() __builtin_extract_return_addr (__builtin_return_address (0))
#else
#define WHODAT() nullptr
#endif
 
#define ALIGNMENT_BITS 3 

//#define TEST_ASSERT_NOT_ON_STACK

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
    SatelliteSerial( ppattern ? ppattern->get() : nullptr, ppattern ),
    asp_pattern( parent_pattern, ppattern )
{
    ASSERT( parent_pattern );
    ASSERT( ppattern );
    ASSERT( *ppattern );
#ifdef KEEP_WHODAT_INFO
    whodat = whodat_ ? whodat_ : WHODAT();
#endif    
    ASSERT_NOT_ON_STACK( ppattern )( *this );
}


PatternLink::PatternLink(const Agent *parent_agent,
                         const TreePtrInterface *ppattern) :
    PatternLink( parent_agent->GetPatternPtr(), ppattern, WHODAT() )
{
}


PatternLink PatternLink::FromWalkIterator( const Walk::iterator &wit, PatternLink root, int generations )
{
    TreePtr<Node> parent_pattern = wit.GetParent(generations);
    const TreePtrInterface *pparent = wit.GetNodePointerInParent(generations);
    ASSERT( root || (parent_pattern && pparent) );
    return parent_pattern ? PatternLink( parent_pattern, pparent ) : root;    
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


size_t PatternLink::GetHash() const noexcept
{
    return std::hash<decltype(asp_pattern)>()(asp_pattern) >> ALIGNMENT_BITS;
}


PatternLink::operator bool() const
{
    return asp_pattern != nullptr;
}


Agent *PatternLink::GetChildAgent() const
{
    ASSERT( asp_pattern )
          ("GetChildAgent() called on uninitialised (nullptr) link\n");
    return Agent::AsAgent((TreePtr<Node>)*asp_pattern);    
}


TreePtr<Node> PatternLink::GetPattern() const
{
    return (TreePtr<Node>)*asp_pattern;
}


const TreePtrInterface *PatternLink::GetPatternPtr() const
{
    return asp_pattern.get();
}


string PatternLink::GetTrace() const
{
    string s = GetName();
#ifdef KEEP_WHODAT_INFO    
    s += SSPrintf("@%lX", (unsigned long)whodat);
#endif
    return s;
}


string PatternLink::GetName() const
{
    string s = GetShortName() + "->";
    if(asp_pattern==nullptr)
        s += "NULL";
    else
        s += GetChildAgent()->GetTrace();
    return s;
}


string PatternLink::GetShortName() const
{
    return GetSerialString();
}


PatternLink::PatternLink(shared_ptr<const TreePtrInterface> ppattern, 
                         void *whodat_) :
    SatelliteSerial( ppattern ? ppattern->get() : nullptr, ppattern.get() ),
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
    SatelliteSerial( px ? px->get() : nullptr, px ),
    asp_x( parent_x, px )              
{
    ASSERT( parent_x );
    ASSERT( px );
    ASSERT( *px );
#ifdef KEEP_WHODAT_INFO
    whodat = whodat_ ? whodat_ : WHODAT();
#endif  
    ASSERT_NOT_ON_STACK( px )( *this );
}


XLink::XLink( const LocatedLink &l ) :
    XLink( l.xlink )              
{
}


XLink XLink::FromWalkIterator( const Walk::iterator &wit, XLink root, int generations )
{
    TreePtr<Node> parent_x = wit.GetParent(generations);
    const TreePtrInterface *px = wit.GetNodePointerInParent(generations);
    ASSERT( root || (parent_x && px) );
    return parent_x ? XLink( parent_x, px, WHODAT() ) : root;    
}

              
XLink XLink::CreateDistinct( const TreePtr<Node> &tp_x )
{
    shared_ptr< TreePtr<Node> > sp_tp_x = make_shared< TreePtr<Node> >( tp_x ); 
    return XLink(sp_tp_x, WHODAT());
}
              
              
bool XLink::operator<(const XLink &other) const
{
    return asp_x < other.asp_x;      
}

    
bool XLink::operator!=(const XLink &other) const
{
    return asp_x != other.asp_x;    
}


bool XLink::operator==(const XLink &other) const
{
    return asp_x == other.asp_x;    
}


size_t XLink::GetHash() const noexcept
{
    return std::hash<decltype(asp_x)>()(asp_x) >> ALIGNMENT_BITS; 
}


XLink::operator bool() const
{
    return asp_x != nullptr;
}


TreePtr<Node> XLink::GetChildX() const
{
    return (TreePtr<Node>)*asp_x;
}


const TreePtrInterface *XLink::GetXPtr() const
{
    return asp_x.get();
}


string XLink::GetTrace() const // used for debug
{
    string s = GetSerialString() + "->";
    if(asp_x==nullptr)
        s += "NULL";
    else
        s += GetChildX()->GetTrace();
#ifdef KEEP_WHODAT_INFO    
    s += SSPrintf("@%lX", (unsigned long)whodat);
#endif
    return s;
}


XLink::XLink( shared_ptr<const TreePtrInterface> px,
              void *whodat_ ) :
    SatelliteSerial( px ? px->get() : nullptr, px.get() ),
    asp_x( px )
{
    ASSERT(px);
    ASSERT(*px);

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
    
#ifdef TEST_ASSERT_NOT_ON_STACK
    // Test the fail case; pass case is being tested all the time anyway
    int a;
    ASSERT_NOT_ON_STACK(&a)(*this);
#endif        
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


size_t LocatedLink::GetHash() const noexcept
{
    return plink.GetHash() + xlink.GetHash();
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


void LocatedLink::SetX( const XLink &xlink_ )
{
    xlink = xlink_;
}


void LocatedLink::SetPattern( const PatternLink &plink_ )
{
    plink = plink_;
}


LocatedLink::operator PatternLink() const
{
    return plink;
}


string LocatedLink::GetTrace() const
{
    return plink.GetTrace() + string(" := ") + xlink.GetTrace();      
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

SolutionMap SR::MapForPattern( const list<PatternLink> &plinks, 
                               const SolutionMap &basic_solution )
{
    SolutionMap m;
    for( PatternLink plink : plinks )
    {
        XLink xlink;
        if( basic_solution.count(plink) > 0 )
        {            
            xlink = basic_solution.at(plink);
        }
        else // missing; probably a SubContainer
        {
            xlink = XLink();
        }
        m[plink] = xlink;
    }
    return m;
}                                      

