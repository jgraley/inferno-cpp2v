#include "common/common.hpp"
#include "common/magic.hpp"
#include "itemise.hpp"
#include "node.hpp"

#include "tree_ptr.hpp"

// To reduce clutter in log diffs when satellite serial numbers are not in synch
//#define SUPPRESS_SATELLITE_NUMBERS

// -------------------------- TreePtrInterface ----------------------------    

TreePtrInterface &TreePtrInterface::operator=( const TreePtrInterface &o )
{
	(void)Itemiser::Element::operator=( o ); 
	return *this;
}


bool TreePtrInterface::operator<(const TreePtrInterface &other) const
{
	return Compare3Way(*this, other) < 0;
}


bool TreePtrInterface::operator==(const TreePtrInterface &other) const
{
	return get() == other.get();
}


bool TreePtrInterface::operator!=(const TreePtrInterface &other) const
{
	return get() != other.get();
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

    // Secondary ordering is on the value of the node (not including subtree) which will
    // help with orderings of sets of things in the trace logs.
    if( Orderable::Diff d_nv = Node::OrderCompare3Way(*l, *r) )
        return d_nv;

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


string TreePtrInterface::GetTrace() const
{
	return GetName();
}    

// -------------------------- TreePtrCommon ----------------------------    

TreePtrCommon::TreePtrCommon()
{
}


TreePtrCommon::TreePtrCommon( Node *o ) :
	SatelliteSerial( o, this )
{
}


TreePtrCommon::TreePtrCommon( nullptr_t o ) :
	SatelliteSerial( nullptr, this )
{
}


const SatelliteSerial &TreePtrCommon::GetSS() const
{
	return *this;
}


string TreePtrCommon::GetName() const
{
	if( !operator bool() )           
		return string("NULL");

#ifdef SUPPRESS_SATELLITE_NUMBERS
	string s = "#?->";
#else
	// Use the serial string of the TreePtr itself #625
	string s = SatelliteSerial::GetSerialString() + "->";
#endif  
	
	s += get()->GetName();
	s += get()->GetSerialString();
	return s;
}  


string TreePtrCommon::GetShortName() const
{
	if( !operator bool() )           
		return string("NULL");

#ifdef SUPPRESS_SATELLITE_NUMBERS
	string s = "#?->";
#else
	// Use the serial string of the TreePtr itself #625
	string s = SatelliteSerial::GetSerialString() + "->";
#endif  
	
	s += get()->GetSerialString(); 
	return s;
}  
