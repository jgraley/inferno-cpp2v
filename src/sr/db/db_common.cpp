#include "db_common.hpp"
#include "common/standard.hpp"

#include <memory>

using namespace SR;

const DBCommon::CoreInfo *DBCommon::GetRootCoreInfo()
{
	return &root_core_info;
}


const DBCommon::CoreInfo *DBCommon::GetUnknownCoreInfo()
{
	return &unknown_core_info;
}


const DBCommon::CoreInfo DBCommon::root_core_info = 
{ 
	TreePtr<Node>(),       // Root don't got no parent                
	-1,					   // Same
	DBCommon::ROOT,        // Has it's            
	nullptr,
	-1,                                                       
	ContainerInterface::iterator() 
};


const DBCommon::CoreInfo DBCommon::unknown_core_info = 
{ 
	TreePtr<Node>(),       
	-1,					
	DBCommon::UNKNOWN,           
	nullptr,
	-1,                                                       
	ContainerInterface::iterator() 
};


DBCommon::RAIISuspendForSwap::RAIISuspendForSwap( DBCommon::TreeOrdinal tree_ordinal1_, TreeZone &zone1_, 
						                          DBCommon::TreeOrdinal tree_ordinal2_, TreeZone &zone2_ ) :
	tree_ordinal1(tree_ordinal1_),
	zone1(zone1_),
	tree_ordinal2(tree_ordinal2_),
	zone2(zone2_)
{
}

				                
DBCommon::RAIISuspendForSwap::~RAIISuspendForSwap()
{
}
