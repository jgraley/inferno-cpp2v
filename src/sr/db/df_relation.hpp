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
		// Siblings mean weakly removed i.e. supplied nodes could be 
		// descendants of the actualk siblings. 
		SAME,
		LEFT_IS_ANCESTOR,
		RIGHT_IS_ANCESTOR,
		CONTAINER_SIBLINGS,
		ITEM_SIBLINGS,
		ROOT_SIBLINGS
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
