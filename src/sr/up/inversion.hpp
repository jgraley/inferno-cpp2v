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
	TreeZoneInverter( Command *initial_cmd, XTreeDatabase *db );
	
	void Run();
	
	shared_ptr<CommandSequence> GetIncrementalSeq();

private:
	typedef pair<XLink, shared_ptr<ZoneExpression> *> LocatedZoneExpression;

	LocatedZoneExpression TryFindFreeZoneExpr(LocatedZoneExpression lze_root);
	void Invert(LocatedZoneExpression lzfe);

	XTreeDatabase * const db;
	UpdateTreeCommand * const root_update_cmd;
	shared_ptr<ZoneExpression> *root_expr;
	const TreeZone root_tree_zone;
	shared_ptr<CommandSequence> incremental_seq;
};

}

#endif
