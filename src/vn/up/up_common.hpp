#ifndef UP_COMMON_HPP
#define UP_COMMON_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../db/duplicate.hpp"
#include "../db/db_common.hpp"
#include <functional>

namespace VN 
{
typedef set<PatternLink> Originators;
typedef map<PatternLink, XLink> ReplaceAssignments;
    
    
// ------------------------- MovesMap --------------------------

struct MoveInfo
{
	TreeZone zone;
	Originators originators;
};

struct MovesMap
{
	map<TreePtr<Node>, MoveInfo> mm;
};

}

#endif
