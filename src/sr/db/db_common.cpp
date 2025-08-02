#include "db_common.hpp"
#include "common/standard.hpp"
#include "tree_zone.hpp"

#include <memory>

using namespace SR;


string Trace(SR::DBCommon::TreeType tt)
{
	switch(tt)
	{
		case SR::DBCommon::TreeType::MAIN:
		return "MAIN";
		
		case SR::DBCommon::TreeType::DOMAIN_EXTENSION:
		return "DOMAIN_EXTENSION";
		
		case SR::DBCommon::TreeType::UPDATE:
		return "UPDATE";
	}
	ASSERTFAIL();
}


string Trace(const SR::DBCommon::TreeRecord &tr) 
{ 
	return "("+Trace(tr.tp_root_node)+", "+
	           Trace(tr.type)+")"; 
}


DBCommon::SwapTransaction::SwapTransaction( TreeZone &zone1_, TreeZone &zone2_ ) :
	zone1(zone1_),
	zone2(zone2_)
{
}

				                
DBCommon::SwapTransaction::~SwapTransaction()
{
}


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

