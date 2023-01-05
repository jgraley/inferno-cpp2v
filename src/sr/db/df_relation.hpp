#ifndef DF_RELATION_HPP
#define DF_RELATION_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "helpers/transformation.hpp"
#include "helpers/walk.hpp"
#include "helpers/flatten.hpp"
#include "../link.hpp"

#include <memory>

namespace SR
{

class XTreeDatabase;

class DepthFirstRelation
{
public:
	enum RelType
	{
		SAME,
		LEFT_IS_ANCESTOR,
		RIGHT_IS_ANCESTOR,
		WEAKLY_REMOVED_CONTAINER_SIBLINGS,
		WEAKLY_REMOVED_ITEM_SIBLINGS,
		ROOTS_ARE_DISTINCT
	};
		
    DepthFirstRelation(const XTreeDatabase *db);

    /// Less operator: for use with set, map etc
    bool operator()( XLink l_xlink, XLink r_xlink ) const;
    Orderable::Diff Compare3Way( XLink l_xlink, XLink r_xlink ) const;
    pair<Orderable::Diff, RelType> CompareHierarchical( XLink l_xlink, XLink r_xlink ) const;

    void Test( const unordered_set<XLink> &xlinks );
    
private:
	const XTreeDatabase * const db;
}; 

};

#endif
