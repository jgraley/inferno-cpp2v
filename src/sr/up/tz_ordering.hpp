#ifndef TZ_ORDERING_HPP
#define TZ_ORDERING_HPP

#include "patches.hpp"
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
	typedef list<shared_ptr<Patch> *> OOOZoneExprPtrList;

	TreeZoneOrderingHandler(XTreeDatabase *db_);
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<Patch> &layout );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<Patch> &layout );

private:	
	struct PatchRecord
	{
		shared_ptr<Patch> *patch_ptr; // Double pointer to patch node so we can mutate
		bool out_of_order; 
	};
	typedef vector<PatchRecord> PatchRecords;

	void RunForTreeZone( shared_ptr<TreeZonePatch> &op, 
						 bool just_check );
	void RunForRange( shared_ptr<Patch> &base, 
                      XLink range_front,
                      XLink range_back,
                      bool just_check );
	void RunForRangeList( PatchRecords &patch_records, 
		    	  	      XLink range_front,
						  XLink range_back,
						  bool just_check );
	void AddTZsBypassingFZs( shared_ptr<Patch> &patch, 
			     	         PatchRecords &patch_records );
	void FindOutOfOrder( PatchRecords &patch_records, 
		    	  	     XLink range_front,
						 XLink range_back,
						 bool just_check );
	shared_ptr<TreeZonePatch> GetTreePatch(const PatchRecord &patch_record) const;
	XLink GetBaseXLink(const PatchRecord &patch_record) const;
				    	  
	XTreeDatabase * const db;
	SR::DepthFirstRelation dfr;		
	OOOZoneExprPtrList out_of_order_list;		    	  
};

// ------------------------- AltTreeZoneOrderingChecker --------------------------

class AltTreeZoneOrderingChecker
{
public:
	AltTreeZoneOrderingChecker(const XTreeDatabase *db_);
	
	void Check( shared_ptr<Patch> layout );

private:
	void Worker( shared_ptr<Patch> patch, bool base_equal_ok );
	void CheckXlink( XLink x, bool equal_ok );
		
	const XTreeDatabase * const db;
	SR::DepthFirstRelation dfr;				    	  
	XLink prev_xlink;
};

}

#endif
