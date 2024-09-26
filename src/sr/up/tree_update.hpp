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
void RunForReplace( const Command *cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db );
	
// ------------------------- TreeZoneOverlapFinder --------------------------

class TreeZoneOverlapFinder : public Traceable
{
public:
	TreeZoneOverlapFinder( const XTreeDatabase *db, shared_ptr<FreeZoneExpression> base );
	
    string GetTrace() const final;

//private:	
    map<const TreeZone *, const PopulateTreeZoneOperator *> tzps_to_commands;
    map<const TreeZone *, set<const TreeZone *>> overlapping_zones;
};

// ------------------------- EmptyZoneElider --------------------------

class EmptyZoneElider
{
public:	
	EmptyZoneElider();
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<FreeZoneExpression> & );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<FreeZoneExpression> & );
};

}

#endif
