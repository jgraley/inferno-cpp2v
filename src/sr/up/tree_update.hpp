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
void RunForReplace( const Command *cmd );
	
// ------------------------- TreeZoneOverlapHandler --------------------------

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
