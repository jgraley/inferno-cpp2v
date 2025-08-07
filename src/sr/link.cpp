#include "link.hpp"
#include "agents/agent.hpp"

// For debugging
#ifdef KEEP_WHODAT_INFO
#define PUSH_WHODAT(W) ((W).push_back(WHODAT()))
#define PUSH_WHODAT_ARG(W, W_) do { (W)=(W_); (W).push_back(WHODAT()); } while(false)
#define PUSH_WHODAT_CMA(W, O) do { (W)=(O).W; (W).push_back(WHODAT()); } while(false)
#define PUSH_WHODAT_CMA_ARG(W, O, W_) do { W=O.W; (W).push_back((W_)?(W_):WHODAT()); } while(false)
#else
#define PUSH_WHODAT(W) {}
#define PUSH_WHODAT_ARG(W, W_) ((void)(W_))
#define PUSH_WHODAT_CMA(W, O) ((void)(O))
#define PUSH_WHODAT_CMA_ARG(W, O, W_) ((void)(O), (void)(W_))
#endif
 
// Tests the not-on-stack tests themseleves
//#define TEST_ASSERT_NOT_ON_STACK

#ifdef TREE_POINTER_REF_COUNTS
#define PLINK_TREE_POINTER_REF_COUNTS
#endif

using namespace SR;

//////////////////////////// PatternLink ///////////////////////////////

PatternLink::PatternLink() :
	p_tpi(nullptr),
    asp_pattern(nullptr)
{
    PUSH_WHODAT(whodat);
}


PatternLink::~PatternLink()
{
#ifdef PLINK_TREE_POINTER_REF_COUNTS
	if( asp_pattern ) 
		asp_pattern->RemoveRef(this);
#endif
}


PatternLink::PatternLink(const PatternLink &other) :
	p_tpi( other.p_tpi ),
    asp_pattern( other.asp_pattern )
{
    PUSH_WHODAT_CMA(whodat, other);

#ifdef PLINK_TREE_POINTER_REF_COUNTS
	if( asp_pattern ) 
		asp_pattern->AddRef(this);
#endif
}


PatternLink &PatternLink::operator=(const PatternLink &other)
{
	ASSERT( &other != this );
	
    PUSH_WHODAT_CMA(whodat, other);

#ifdef PLINK_TREE_POINTER_REF_COUNTS
	if( asp_pattern ) 
		asp_pattern->RemoveRef(this);
#endif
	p_tpi = other.p_tpi;
	asp_pattern = other.asp_pattern;	
	
#ifdef PLINK_TREE_POINTER_REF_COUNTS
	if( asp_pattern )
		asp_pattern->AddRef(this);	
#endif
	return *this;
}


PatternLink::PatternLink(shared_ptr<const Node> parent_pattern,
                         const TreePtrInterface *p_tpi_, 
                         Whodat whodat_) :
	p_tpi( p_tpi_ ),
    asp_pattern( parent_pattern, p_tpi_ )
{
    ASSERT( parent_pattern );
    ASSERT( p_tpi_ );
    ASSERT( *p_tpi_ );
	PUSH_WHODAT_ARG( whodat, whodat_ );
    ASSERT_NOT_ON_STACK( p_tpi_ )( *this );
#ifdef PLINK_TREE_POINTER_REF_COUNTS
	asp_pattern->AddRef(this);
#endif	    
}


PatternLink::PatternLink(shared_ptr<const TreePtrInterface> ppattern, 
                         Whodat whodat_) :
	p_tpi( ppattern.get() ),
    asp_pattern( ppattern )
{
	PUSH_WHODAT_ARG( whodat, whodat_ );    
#ifdef PLINK_TREE_POINTER_REF_COUNTS
	if( asp_pattern )
		asp_pattern->AddRef(this);	
#endif
}


PatternLink::PatternLink(const Agent *parent_agent,
                         const TreePtrInterface *ppattern) :
    PatternLink( parent_agent->GetPatternPtr(), ppattern, {WHODAT()} )
{
}


PatternLink PatternLink::CreateDistinct( const TreePtr<Node> &tp_pattern )
{
    shared_ptr< TreePtr<Node> > sp_tp_pattern = make_shared< TreePtr<Node> >( tp_pattern ); 
    return PatternLink(sp_tp_pattern, {WHODAT()});
}
              
              
bool PatternLink::operator<(const PatternLink &other) const
{
    return Compare3Way( *this, other ) < 0;
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


Orderable::Diff PatternLink::Compare3Way(const PatternLink &l, const PatternLink &r)
{
    // NULLness is primary ordering because we wish to dereference both pointers. 
    // If both are NULL we'll call that equal, and drop out. Also do fast-out on 
    // equal pointers.
    if( r.asp_pattern==l.asp_pattern )
        return 0; // for == case
    else if( !r.asp_pattern )
        return 1; // for > case
    else if( !l.asp_pattern )
        return -1; // for < case
    
    // Secondary ordering is on the value of the TreePtr which will
    // help with orderings of sets of things in the trace logs.
    if( Orderable::Diff d_node = TreePtrInterface::Compare3Way( *l.asp_pattern, *r.asp_pattern ) )
        return d_node;

    // Tertiary ordering is on the identities of the TreePtrs, which 
    // corresponds to the values of the PatternLinks.
    if( Orderable::Diff d_tpi = TreePtrInterface::Compare3WayIdentity( *l.asp_pattern, *r.asp_pattern ) )
        return d_tpi;
       
    return 0;
}


size_t PatternLink::GetHash() const noexcept
{
    return std::hash<decltype(asp_pattern)>()(asp_pattern) >> HASHING_POINTERS_ALIGNMENT_BITS;
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
Mutate() on the container.
#endif
}


string PatternLink::GetTrace() const
{
    if(asp_pattern==nullptr)
        return "NULL";
    string s = "↳"+asp_pattern->GetTrace();
#ifdef KEEP_WHODAT_INFO    
    s += Trace(whodat);
#endif
    return s;
}


string PatternLink::GetName() const
{
    if(asp_pattern==nullptr)
        return "NULL";
    return "↳"+asp_pattern->GetName();
}


string PatternLink::GetShortName() const
{
    if(asp_pattern==nullptr)
        return "NULL";
	// Serial string included because needed in graphs to disambiguate links
    string s = "↳" + asp_pattern->GetSerialString() + "->" + asp_pattern->GetShortName();
    // I think we should be calling eg GetSerialString() from *outside* obejcts, i.e.
    // from pointer-liuke objects, since the awrial number is not a part of the
    // object's value.
    return s;
}

//////////////////////////// XLink ///////////////////////////////

XLink::XLink() :
    p_tpi( nullptr )
{
    PUSH_WHODAT(whodat);
}


XLink::~XLink() 
{
#ifdef XLINK_TREE_POINTER_REF_COUNTS
	if( p_tpi ) 
		p_tpi->RemoveRef(this);
#endif
}


XLink::XLink(const XLink &other) :
    p_tpi( other.p_tpi )
{
    PUSH_WHODAT_CMA(whodat, other);

#ifdef XLINK_TREE_POINTER_REF_COUNTS
	if( p_tpi ) 
		p_tpi->AddRef(this);
#endif
}


XLink &XLink::operator=(const XLink &other)
{
	ASSERT( &other != this );
	
    PUSH_WHODAT_CMA(whodat, other);

#ifdef XLINK_TREE_POINTER_REF_COUNTS
	if( p_tpi ) 
		p_tpi->RemoveRef(this);
#endif
	p_tpi = other.p_tpi;
	
#ifdef XLINK_TREE_POINTER_REF_COUNTS
	if( p_tpi )
		p_tpi->AddRef(this);	
#endif
	return *this;
}


XLink::XLink( const TreePtrInterface *p_tpi_,
              Whodat whodat_ ) :
    p_tpi( p_tpi_ )
{
	PUSH_WHODAT_ARG( whodat, whodat_ );
    ASSERT( p_tpi );
    ASSERT( *p_tpi );
    ASSERT_NOT_ON_STACK( p_tpi_ )( *this );
#ifdef XLINK_TREE_POINTER_REF_COUNTS
	p_tpi->AddRef(this);
#endif	
}


XLink::XLink( const LocatedLink &l ) :
    XLink( l.xlink )              
{
}

              
XLink XLink::CreateFrom( const TreePtrInterface *p_tpi,
                         Whodat whodat_ )
{
    return XLink(p_tpi, whodat_);
}


size_t XLink::GetHash() const noexcept
{
    return std::hash<decltype(p_tpi)>()(p_tpi) >> HASHING_POINTERS_ALIGNMENT_BITS; 
}


XLink::operator bool() const
{
    return p_tpi != nullptr;
}


bool XLink::HasChildX() const
{
    return !!*p_tpi;
}


TreePtr<Node> XLink::GetChildTreePtr() const
{
	//FTRACE(p_tpi)("\n");
    return (TreePtr<Node>)*p_tpi;
}


const TreePtrInterface *XLink::GetTreePtrInterface() const
{
    return p_tpi;
}


#ifdef KEEP_WHODAT_INFO
XLink::Whodat XLink::GetWhodat() const
{
	return whodat;
}
#endif	

string XLink::GetTrace() const
{
    if(p_tpi==nullptr)
        return "NULL";
    string s = "⤷"+p_tpi->GetTrace();
#ifdef KEEP_WHODAT_INFO    
    s += Trace(whodat);
#endif
    return s;
}


string XLink::GetName() const 
{
    if(p_tpi==nullptr)
        return "NULL";
    return "⤷"+p_tpi->GetName();
}


string XLink::GetShortName() const 
{
    if(p_tpi==nullptr)
        return "NULL";
	
	// Serial string included because needed in graphs to disambiguate links
    string s = "⤷" + p_tpi->GetSerialString() + "->" + p_tpi->GetShortName();
    // I think we should be calling eg GetSerialString() from *outside* obejcts, i.e.
    // from pointer-like objects, since the awrial number is not a part of the
    // object's value.
    return s;    
}

const TreePtr<XLink::MMAXNodeType> XLink::MMAXNode = MakeTreeNode<XLink::MMAXNodeType>();
const XLink XLink::MMAX = XLink::CreateFrom( &XLink::MMAXNode );

const TreePtr<XLink::OffEndNodeType> XLink::OffEndNode = MakeTreeNode<XLink::OffEndNodeType>();
const XLink XLink::OffEnd = XLink::CreateFrom( &XLink::OffEndNode );
              
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


TreePtr<Node> LocatedLink::GetChildTreePtr() const
{
    return xlink.GetChildTreePtr();
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

