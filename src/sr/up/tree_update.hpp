#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "free_zone.hpp"
#include "tree_zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"
#include "ssa_allocator.hpp"

namespace SR 
{

class DeclareTreeZoneCommand;

// ------------------------- Runners --------------------------

FreeZone RunForBuilder( unique_ptr<Command> cmd );
void RunForReplace( unique_ptr<Command> cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db );
	
// ------------------------- TreeZoneOverlapFinder --------------------------

class TreeZoneOverlapFinder
{
public:
	TreeZoneOverlapFinder( const XTreeDatabase *db, CommandSequence *seq );
	
private:	
    map<const TreeZone *, const DeclareTreeZoneCommand *> tzps_to_commands;
    map<const TreeZone *, set<const TreeZone *>> overlapping_zones;
};

// ------------------------- CommandSequenceFlattener --------------------------

class CommandSequenceFlattener
{
public:
	void Apply( CommandSequence &seq );
	void Worker( CommandSequence &seq, list<unique_ptr<Command>> &commands );
};

// ------------------------- EmptyZoneRemover --------------------------

class EmptyZoneRemover
{
public:
	struct OperandMaps
	{
		map<SSAAllocator::Reg, set<unique_ptr<Command> *>> as_source;
		map<SSAAllocator::Reg, set<unique_ptr<Command> *>> as_target;
		map<SSAAllocator::Reg, unique_ptr<Command> *> as_dest; // SSA so only one		
	};
	OperandMaps DetermineOpUsers( CommandSequence &seq );
	void Apply( CommandSequence &seq );
};

}

#endif
