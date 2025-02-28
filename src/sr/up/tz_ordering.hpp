#ifndef TZ_ORDERING_HPP
#define TZ_ORDERING_HPP

#include "zone_expressions.hpp"
#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// ------------------------- TreeZoneOrderingHandler --------------------------

class TreeZoneOrderingHandler
{
public:	
	typedef list<shared_ptr<Layout> *> OOOZoneExprPtrList;

	TreeZoneOrderingHandler(const XTreeDatabase *db_);
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<Layout> &root_expr );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<Layout> &root_expr );

private:	
	struct Thing
	{
		shared_ptr<Layout> *expr_ptr; // Double pointer to expr node so we can mutate
		bool out_of_order; 
	};
	typedef vector<Thing> ThingVector;

	void RunForTreeZone( shared_ptr<DupMergeTreeZoneOperator> &op, 
						 bool just_check );
	void RunForRange( shared_ptr<Layout> &base, 
                      XLink range_last,
                      XLink range_end,
                      bool just_check );
	void RunForRangeList( ThingVector &things, 
		    	  	      XLink range_first,
						  XLink range_last,
						  bool just_check );
	void AddTZsBypassingFZs( shared_ptr<Layout> &expr, 
			     	         ThingVector &tree_zones );
	void FindOutOfOrder( ThingVector &things, 
		    	  	     XLink range_first,
						 XLink range_last,
						 bool just_check );
	shared_ptr<DupMergeTreeZoneOperator> GetOperator(const Thing &thing) const;
	XLink GetBaseXLink(const Thing &thing) const;
				    	  
	const XTreeDatabase * const db;
	SR::DepthFirstRelation dfr;		
	OOOZoneExprPtrList out_of_order_list;		    	  
};

// ------------------------- AltTreeZoneOrderingChecker --------------------------

class AltTreeZoneOrderingChecker
{
public:
	AltTreeZoneOrderingChecker(const XTreeDatabase *db_);
	
	void Check( shared_ptr<Layout> root_expr );

private:
	void Worker( shared_ptr<Layout> expr, bool base_equal_ok );
	void CheckXlink( XLink x, bool equal_ok );
		
	const XTreeDatabase * const db;
	SR::DepthFirstRelation dfr;				    	  
	XLink prev_xlink;
};

}

#endif
