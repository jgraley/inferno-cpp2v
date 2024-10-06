#ifndef INVERSION_HPP
#define INVERSION_HPP

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

// Inversi works on commands, not expressions
class TreeZoneInverter 
{
public:
	TreeZoneInverter( const Command *initial_cmd, const XTreeDatabase *db );
	
	void Run();
	
	const CommandSequence &GetSplitSeq();

private:
	bool TryInvertOne();

	const XTreeDatabase * const db;
	const UpdateTreeCommand * const root_update_cmd;
	const shared_ptr<ZoneExpression> root_expr;
	const TreeZone root_tree_zone;
	CommandSequence split_seq;
};

}

#endif
