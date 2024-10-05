#ifndef TZ_ORDERING_HPP
#define TZ_ORDERING_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "free_zone.hpp"
#include "tree_zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"
#include "commands.hpp"

namespace SR 
{

class DeclareTreeZoneCommand;

// ------------------------- TreeZoneOrderingHandler --------------------------

class TreeZoneOrderingHandler
{
public:	
	TreeZoneOrderingHandler(const XTreeDatabase *db_);
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<ZoneExpression> &root_expr );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<ZoneExpression> &root_expr );

private:	
	void RunWorker( shared_ptr<ZoneExpression> &base, 
                    XLink range_begin,
                    XLink range_end,
                    bool just_check );
	void RunForTreeZone( shared_ptr<PopulateTreeZoneOperator> &op, 
						 bool just_check );
	void GatherTreeZoneOps( shared_ptr<ZoneExpression> &expr, 
				      	    list<shared_ptr<ZoneExpression> *> &tree_zones );
	void DuplicateTreeZone( shared_ptr<ZoneExpression> &expr );
				    	  
	const XTreeDatabase * const db;
	SR::DepthFirstRelation dfr;				    	  
};

// ------------------------- AltTreeZoneOrderingChecker --------------------------

class AltTreeZoneOrderingChecker
{
public:
	AltTreeZoneOrderingChecker(const XTreeDatabase *db_);
	
	void Check( shared_ptr<ZoneExpression> root_expr );

private:
	void Worker( shared_ptr<ZoneExpression> expr, bool base_equal_ok );
	void CheckXlink( XLink x, bool equal_ok );
		
	const XTreeDatabase * const db;
	SR::DepthFirstRelation dfr;				    	  
	XLink prev_xlink;
};

}

#endif
