#ifndef UP_UTILS_HPP
#define UP_UTILS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{
// ------------------------- EmptyZoneElider --------------------------

class EmptyZoneElider
{
public:	
	EmptyZoneElider();
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<ZoneExpression> &root_expr );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<ZoneExpression> &root_expr );
};

// ------------------------- BaseForEmbeddedMarkPropagation --------------------------

class BaseForEmbeddedMarkPropagation 
{
public:
	BaseForEmbeddedMarkPropagation( const XTreeDatabase *db );
	void Run( shared_ptr<ZoneExpression> &root_expr );
	void Check( shared_ptr<ZoneExpression> &root_expr );

private:
	const XTreeDatabase * const db;
};

// ------------------------- DuplicateAllToFree --------------------------

class DuplicateAllToFree
{
public:	
	DuplicateAllToFree();
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<ZoneExpression> &root_expr );
	
	// Just ASSERT all free zones
	void Check( shared_ptr<ZoneExpression> &root_expr );
};

}

#endif
