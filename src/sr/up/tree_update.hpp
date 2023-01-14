#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

class ImmediateTreeZoneCommand;

// ------------------------- Runners --------------------------

FreeZone RunGetFreeZoneNoDB( unique_ptr<Command> cmd, const SCREngine *scr_engine );
void RunVoidForReplace( unique_ptr<Command> cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db );
	
// ------------------------- TreeZoneOverlapFinder --------------------------

class TreeZoneOverlapFinder
{
public:
	TreeZoneOverlapFinder( const XTreeDatabase *db, CommandSequence *seq );
	
private:	
    map<const TreeZone *, const ImmediateTreeZoneCommand *> tzps_to_commands;
    map<const TreeZone *, set<const TreeZone *>> overlapping_zones;
};

// ------------------------- CommandSequenceFlattener --------------------------

class CommandSequenceFlattener
{
public:
	void Apply( CommandSequence &seq );
	void Worker( CommandSequence &seq, list<unique_ptr<Command>> &commands );
};

}

#endif
