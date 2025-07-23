#include "common/common.hpp"
#include "common/magic.hpp"
#include "itemise.hpp"
#include "node.hpp"

#include "tree_ptr.hpp"

// To reduce clutter in log diffs when satellite serial numbers are not in synch
#define SUPPRESS_SATELLITE_NUMBERS

// Ordering on node is not a requirement for repeatability. I think it's only there to
// keep logs nice by grouping similar nodes. But it slows down set<TreePtr<>> and
// set<XLink> quite a lot.
//#define TREE_PTR_ORDERED_ON_NODE


// -------------------------- TreePtrInterface ----------------------------    

TreePtrInterface &TreePtrInterface::operator=( const TreePtrInterface &o )
{
    (void)Itemiser::Element::operator=( o ); 
    return *this;
}


bool TreePtrInterface::operator<(const TreePtrInterface &r) const
{
    return get() < r.get();
}


bool TreePtrInterface::operator==(const TreePtrInterface &r) const
{
    return get() == r.get();
}


bool TreePtrInterface::operator!=(const TreePtrInterface &r) const
{
    return get() != r.get();
}


Orderable::Diff TreePtrInterface::Compare3Way(const TreePtrInterface &l, const TreePtrInterface &r)
{
    // NULLness is primary ordering because we wish to dereference both pointers. 
    // If both are NULL we'll call that equal, and drop out. Also do fast-out on 
    // equal pointers.
    if( l==r )
        return 0; // for == case
    else if( !r )
        return 1; // for > case
    else if( !l )
        return -1; // for < case        
    ASSERTS( l );
    ASSERTS( r );

#ifdef TREE_PTR_ORDERED_ON_NODE
    // Secondary ordering is on the value of the node (not including subtree) which will
    // help with orderings of sets of things in the trace logs.
    if( Orderable::Diff d_nv = Node::OrderCompare3Way( *l, *r ) )
        return d_nv;
#endif

    // Tertiary ordering is on the identities of the nodes, which corresponds to the values of 
    // the TreePtrs.
    if( Orderable::Diff d_ni = Node::Compare3WayIdentity( *l, *r ) )
        return d_ni;
        
    // Check that last claim
    ASSERTS( l==r );
    return 0;
}


Orderable::Diff TreePtrInterface::Compare3WayIdentity(const TreePtrInterface &l, const TreePtrInterface &r)
{
    return SatelliteSerial::Compare3WayIdentity( l.GetSS(), r.GetSS() );
}

// -------------------------- TreePtrCommon ----------------------------    

TreePtrCommon::TreePtrCommon()
{
}


#ifdef TREE_POINTER_REF_COUNTS
TreePtrCommon::~TreePtrCommon()
{
	// Use ASSERT for static since we're in a destructor and our concrete class has gone away
	ASSERTS( ref_count==0 )
	       ("Ref count at delete: %u\n", ref_count)
#ifdef TREE_POINTER_REF_TRACKING
	       (references)
#endif
           ;
}
#endif


TreePtrCommon::TreePtrCommon( Node *o ) :
    SatelliteSerial( o, this )
{
}


TreePtrCommon::TreePtrCommon( nullptr_t ) :
    SatelliteSerial( nullptr, this )
{
}


TreePtrCommon::TreePtrCommon(const TreePtrCommon &other) :
	Itemiser::Element( other ), // Itemiser requires that itemisable's operator= calls though to Element.
    SatelliteSerial( other )
{
	// but not the refs since this is a new instance
}


TreePtrCommon &TreePtrCommon::operator=(const TreePtrCommon &other)
{	
	Itemiser::Element::operator=(other);
	SatelliteSerial::operator=(other);
	// but not the refs since this is a different instance
	return *this;
}


#ifdef TREE_POINTER_REF_COUNTS
void TreePtrCommon::AddRef(const Traceable *ref) const
{ 
	//TRACE("Add ref ")(ref)("\n");
	
	ref_count++; 
	
#ifdef TREE_POINTER_REF_TRACKING
	ASSERT( references.count(ref)==0 )
	      ("Ref: ")
	      (ref)
	      (" added more than once\n")
	      (references);
	InsertSolo( references, ref );
#endif	
}


void TreePtrCommon::RemoveRef(const Traceable *ref) const 
{ 
	//TRACE("Remove ref ")(ref)("\n");
	ASSERT( ref_count>0 )
	      ("Ref count decrement past zero\n")
#ifdef TREE_POINTER_REF_TRACKING
	       (references)
#endif
           ;
           
	ref_count--; 
	
#ifdef TREE_POINTER_REF_TRACKING
	EraseSolo( references, ref );
#endif	
}    
#endif



const SatelliteSerial &TreePtrCommon::GetSS() const
{
    return *this;
}


string TreePtrCommon::GetName() const
{
	// This is used by graph plotting, and must describe the tree pointer itself, 
	// not the pointed-to node!
    string s = Traceable::GetName();

#ifndef SUPPRESS_SATELLITE_NUMBERS
    // Use the serial string of the TreePtr itself #625
    s += SatelliteSerial::GetSerialString();
#endif  
    return s;
}  


string TreePtrCommon::GetShortName() const
{
    if( !operator bool() )           
        return string("NULL");

#ifdef SUPPRESS_SATELLITE_NUMBERS
    string s = "&";
#else
    // Use the serial string of the TreePtr itself #625
    string s = SatelliteSerial::GetSerialString() + "->";
#endif  
    
    s += get()->GetSerialString(); 
    return s;
}  


string TreePtrCommon::GetTrace() const
{
#ifdef SUPPRESS_SATELLITE_NUMBERS
	string s = "&";
#else
	// Use the serial string of the TreePtr itself #625
	string s = SatelliteSerial::GetSerialString() + "->";
#endif  
	// Don't use vcalls because could be invoked in destructor
    return "TreePtrCommon";
}    
