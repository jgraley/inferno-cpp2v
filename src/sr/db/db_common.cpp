#include "db_common.hpp"
#include "common/standard.hpp"

#include <memory>

using namespace SR;

const DBCommon::CoreInfo *DBCommon::GetRootCoreInfo()
{
	return &root_core_info;
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


DBCommon::RAIISuspendForSwap::RAIISuspendForSwap( DBCommon::TreeOrdinal tree_ordinal1_, TreeZone &zone1_, const DBCommon::CoreInfo *base_info1_,
						                          DBCommon::TreeOrdinal tree_ordinal2_, TreeZone &zone2_, const DBCommon::CoreInfo *base_info2_ ) :
	tree_ordinal1(tree_ordinal1_),
	zone1(zone1_),
	base_info1(base_info1_),
	tree_ordinal2(tree_ordinal2_),
	zone2(zone2_),
	base_info2(base_info2_)
{
}

				                
DBCommon::RAIISuspendForSwap::~RAIISuspendForSwap()
{
}
