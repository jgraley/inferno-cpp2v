#ifndef DB_COMMON_HPP
#define DB_COMMON_HPP

#include "../link.hpp"
#include "common/standard.hpp"

namespace SR 
{
class DBCommon
{
public:  
    typedef int OrdinalType; // other ordinals...

	enum class RootOrdinal
	{
		MAIN,
		MMAX,
		OFF_END,
		EXTRAS // EXTRAS+i
	};

	struct RootRecord
	{
		RootOrdinal ordinal;
	};    
};    
    
}

#endif
