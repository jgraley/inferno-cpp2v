#ifndef TZ_ORDERING_HPP
#define TZ_ORDERING_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"
#include "zone_commands.hpp"

namespace SR 
{

// ------------------------- TreeZoneOrderingHandler --------------------------

class TreeZoneOrderingHandler
{
public:	
	typedef list<shared_ptr<ZoneExpression> *> OOOZoneExprPtrList;

	TreeZoneOrderingHandler(const XTreeDatabase *db_);
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<ZoneExpression> &root_expr );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<ZoneExpression> &root_expr );

private:	
	typedef list<shared_ptr<ZoneExpression> *> ZoneExprPtrList;

	void RunForTreeZone( shared_ptr<DupMergeTreeZoneOperator> &op, 
						 bool just_check );
	void RunForRange( shared_ptr<ZoneExpression> &base, 
                      XLink range_last,
                      XLink range_end,
                      bool just_check );
	void InsertTZsBypassingFZs( shared_ptr<ZoneExpression> &expr, 
			     	            ZoneExprPtrList &tree_zones,
				      	        ZoneExprPtrList::iterator pos );
				    	  
	const XTreeDatabase * const db;
	SR::DepthFirstRelation dfr;		
	OOOZoneExprPtrList out_of_order_list;		    	  
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
