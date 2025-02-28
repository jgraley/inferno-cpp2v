#ifndef INVERSION_HPP
#define INVERSION_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// Inversi works on commands, not expressions
class TreeZoneInverter 
{
public:
	TreeZoneInverter( XTreeDatabase *db );
	
	void Run(TreeZone root_target, shared_ptr<ZoneExpression> *source_layout_ptr);
	
private:
	// .first is NULL: base not known
	typedef pair<XLink, shared_ptr<ZoneExpression> *> LocatedZoneExpression;

	void WalkFreeZoneExpr(LocatedZoneExpression lze_root);
	void Invert(LocatedZoneExpression lzfe);

	XTreeDatabase * const db;
};

}

#endif
