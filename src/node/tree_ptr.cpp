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
    // Half-NULLness is super-primary ordering
    if( !r && !l )
        return 0; // for == case
    else if( !r )
        return 1; // for > case
    else if( !l )
        return -1; // for < case
        
    ASSERTS( l );
    ASSERTS( r );

	return Node::Compare3WayIdentity( *l, *r );
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
