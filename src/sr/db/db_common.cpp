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
