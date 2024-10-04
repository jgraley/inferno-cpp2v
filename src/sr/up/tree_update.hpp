#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

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

// ------------------------- Runners --------------------------

FreeZone RunForBuilder( const FreeZoneExpression *expr );
void RunForReplace( const Command *cmd, XTreeDatabase *x_tree_db );
	
// ------------------------- TreeZoneOverlapHandler --------------------------

// To optimise:
// - Make a std::set/multiset of TZs ordered using TreeZoneRelation (idea 
//   is that the order in the replace expression is irrelevent to this algo)
// - Add a GetLastDescendant() so can form ranges for descendants which
//   are the overlap candidates.
// - TreeZoneRelation is secondary on terminii. Get matching base ranges 
//   by just depth-first-incrementing the base for an exclusive end.
// - Identical TZs are equal in TreeZoneRelation so if regular std::set 
//   is used, they will be dropped and if std::multiset they will be kept.

class TreeZoneOverlapHandler 
{
public:
	TreeZoneOverlapHandler( const XTreeDatabase *db );
	void Run( shared_ptr<FreeZoneExpression> &base );
	void Check( shared_ptr<FreeZoneExpression> &base );

private:
	const XTreeDatabase * const db;
};

// ------------------------- EmptyZoneElider --------------------------

class EmptyZoneElider
{
public:	
	EmptyZoneElider();
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<FreeZoneExpression> &base );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<FreeZoneExpression> &base );
};

// ------------------------- FreeZoneMerger --------------------------

class FreeZoneMerger
{
public:	
	FreeZoneMerger();
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<FreeZoneExpression> &base );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<FreeZoneExpression> &base );
};

// ------------------------- TreeZoneOrderingHandler --------------------------

class TreeZoneOrderingHandler
{
public:	
	TreeZoneOrderingHandler(const XTreeDatabase *db_);
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<FreeZoneExpression> &base );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<FreeZoneExpression> &base );

private:	
	void RunWorker( shared_ptr<FreeZoneExpression> &base, 
                    XLink range_begin,
                    XLink range_end,
                    bool just_check );
	void RunForTreeZone( shared_ptr<PopulateTreeZoneOperator> &op, 
						 bool just_check );
	void GatherTreeZoneOps( shared_ptr<FreeZoneExpression> &expr, 
				      	    list<shared_ptr<FreeZoneExpression> *> &tree_zones );
	void DuplicateTreeZone( shared_ptr<FreeZoneExpression> &expr );
				    	  
	const XTreeDatabase * const db;
	SR::DepthFirstRelation dfr;				    	  
};


}

#endif
