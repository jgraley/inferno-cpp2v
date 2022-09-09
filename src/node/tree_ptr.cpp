#include "common/common.hpp"
#include "common/magic.hpp"
#include "itemise.hpp"
#include "node.hpp"

#include "tree_ptr.hpp"

TreePtrInterface &TreePtrInterface::operator=( const TreePtrInterface &o )
{
	(void)Itemiser::Element::operator=( o ); 
	(void)Traceable::operator=( o );
	return *this;
}


bool TreePtrInterface::operator<(const TreePtrInterface &other) const
{
	return GetSS() < other.GetSS();
}


bool TreePtrInterface::operator==(const TreePtrInterface &other) const
{
	return get() == other.get();
}


bool TreePtrInterface::operator!=(const TreePtrInterface &other) const
{
	return get() != other.get();
}


string TreePtrInterface::GetTrace() const
{
	return GetName();
}    


TreePtrCommon::TreePtrCommon()
{
}


TreePtrCommon::TreePtrCommon( Node *o ) : // dangerous - make explicit
	SatelliteSerial( o, this )
{
}


TreePtrCommon::TreePtrCommon( nullptr_t o ) : // safe - leave implicit
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
