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

//#define SUPPRESS_SATELLITE_NUMBERS_XLINK

// Tests the not-on-stack tests themseleves
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


PatternLink PatternLink::CreateDistinct( const TreePtr<Node> &tp_pattern )
{
    shared_ptr< TreePtr<Node> > sp_tp_pattern = make_shared< TreePtr<Node> >( tp_pattern ); 
    return PatternLink(sp_tp_pattern, WHODAT());
}
              
              
bool PatternLink::operator<(const PatternLink &other) const
{
    ASSERT( this );
    ASSERT( &other );
    // PatternLink is unique across parent-child links in
    // the pattern. This operator will permit PatternLink to 
    // act as keys in maps.
        
    // NULLness is super-primary ordering
    if( !other.asp_pattern )
        return false; // for == and > case
    else if( !asp_pattern )
        return true; // for remaining < case
    ASSERT( *asp_pattern );
    ASSERT( *other.asp_pattern );
    
    // Child node serial number is primary ordering
    auto tp_this = TreePtr<Node>(*asp_pattern);
    auto tp_other = TreePtr<Node>(*other.asp_pattern);
    ASSERT( tp_this );
    ASSERT( tp_other );
    if( tp_this->Node::operator<(*tp_other) )
        return true;
    if( tp_other->Node::operator<(*tp_this) )
        return false;

    // Satellite serial number aka arrow-head number is secondary ordering
    // Use ordering on the TreePtrs themselves #625
    if( asp_pattern->TreePtrInterface::operator<(*other.asp_pattern) )
        return true;
    if( other.asp_pattern->TreePtrInterface::operator<(*asp_pattern) )
        return false;
       
    // Pointer-based tertiary ordering for just in case TODO assert pointers are equal
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


const TreePtrInterface *PatternLink::GetPatternTreePtr() const
{
    return asp_pattern.get();
}


void PatternLink::Redirect( const TreePtrInterface &new_parent_pattern )
{
#if USE_LIST_FOR_COLLECTION
    // With this setting, collections as well as sequences are really
    // lists, and the const cast is safe. In fact the constness of 
    // associative keys is the only reason for asp_pattern pointing
    // to const.
    *const_pointer_cast<TreePtrInterface>(asp_pattern) = new_parent_pattern;
#else
#error If associative containers are to be used in nodes, the const \
cast wont be safe and this function needs to be able to call \
Overwrite() on the container.
#endif
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
    if(asp_pattern==nullptr)
        return "NULL";
    return asp_pattern->GetName();
}


string PatternLink::GetShortName() const
{
    if(asp_pattern==nullptr)
        return "NULL";
    return asp_pattern->GetShortName();
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
    ASSERT( parent_x );
    ASSERT( px );
#ifdef KEEP_WHODAT_INFO
    whodat = whodat_ ? whodat_ : WHODAT();
#endif  
    ASSERT_NOT_ON_STACK( px )( *this );
}


XLink::XLink( const LocatedLink &l ) :
    XLink( l.xlink )              
{
}


XLink XLink::CreateDistinct( const TreePtr<Node> &tp_x )
{
    auto sp_tp_x = make_shared< TreePtr<Node> >( tp_x ); 
    return XLink(sp_tp_x, WHODAT());
}
              
              
bool XLink::operator<(const XLink &other) const
{
    ASSERT( this );
    ASSERT( &other );
    
    // NULLness is hyper-primary ordering
    if( !other.asp_x )
        return false; // for == and > case
    else if( !asp_x )
        return true; // for remaining < case
    
    // Half-NULLness is super-primary ordering
    if( !*other.asp_x )
        return false; // for == and > case
    else if( !*asp_x )
        return true; // for remaining < case
        
    ASSERT( *asp_x );
    ASSERT( *other.asp_x );
    
    // Child node serial number is primary ordering
    auto tp_this = TreePtr<Node>(*asp_x);
    auto tp_other = TreePtr<Node>(*other.asp_x);
    ASSERT( tp_this );
    ASSERT( tp_other );
    if( tp_this->Node::operator<(*tp_other) )
        return true;
    if( tp_other->Node::operator<(*tp_this) )
        return false;

    // Satellite serial number aka arrow-head number is secondary ordering
    // Use ordering on the TreePtrs themselves #625
    if( asp_x->TreePtrInterface::operator<(*other.asp_x) )
        return true;
    if( other.asp_x->TreePtrInterface::operator<(*asp_x) )
        return false;
       
    // Pointer-based tertiary ordering for just in case TODO assert pointers are equal
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


bool XLink::HasChildX() const
{
    return !!*asp_x;
}


TreePtr<Node> XLink::GetChildX() const
{
    return (TreePtr<Node>)*asp_x;
}


const TreePtrInterface *XLink::GetXPtr() const
{
    return asp_x.get();
}


void XLink::SetXPtr(const TreePtrInterface &x)
{
	// TODO try to get rid of the const cast
	const_cast<TreePtrInterface &>(*asp_x) = x;
}


string XLink::GetTrace() const
{
    string s = GetName();
#ifdef KEEP_WHODAT_INFO    
    s += SSPrintf("@%lX", (unsigned long)whodat);
#endif
    return s;
}


string XLink::GetName() const 
{
    if(asp_x==nullptr)
        return "NULL";
    return "&"+asp_x->GetName();
}


string XLink::GetShortName() const 
{
    if(asp_x==nullptr)
        return "NULL";
    return "&"+asp_x->GetShortName();
}


XLink::XLink( shared_ptr<const TreePtrInterface> px,
              void *whodat_ ) :
    asp_x( px )
{
    ASSERT(px);

#ifdef KEEP_WHODAT_INFO
    whodat = whodat_ ? whodat_ : WHODAT();
#endif  
}              


const XLink XLink::MMAX_Link = XLink::CreateDistinct( MakeTreeNode<XLink::MMAX>() );
const XLink XLink::OffEndXLink = XLink::CreateDistinct( MakeTreeNode<XLink::OffEnd>() );
              
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


const TreePtrInterface *LocatedLink::GetPatternTreePtr() const
{
    return plink.GetPatternTreePtr();
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
    return plink.GetName() + string(" := ") + xlink.GetTrace(); // Whodat info comes from clink     
}


string LocatedLink::GetName() const
{
    return plink.GetName() + string(" := ") + xlink.GetName();      
}


string LocatedLink::GetShortName() const 
{
    return plink.GetShortName() + string(" := ") + xlink.GetShortName();      
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

