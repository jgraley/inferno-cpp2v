#ifndef INVERSION_HPP
#define INVERSION_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"
#include "zone_commands.hpp"

namespace SR 
{

// Inversi works on commands, not expressions
class TreeZoneInverter 
{
public:
	TreeZoneInverter( XTreeDatabase *db );
	
	shared_ptr<Command> Run(shared_ptr<Command> initial_cmd);
	
private:
	// .first is NULL: base not known
	typedef pair<XLink, shared_ptr<ZoneExpression>> LocatedZoneExpression;

	void WalkFreeZoneExpr(LocatedZoneExpression lze_root);
	void Invert(LocatedZoneExpression lzfe);

	XTreeDatabase * const db;
	shared_ptr<CommandSequence> incremental_seq;
};

}

#endif
