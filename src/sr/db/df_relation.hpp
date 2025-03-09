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
	typedef XLink KeyType;

	enum RelType
	{
		// Siblings mean weakly removed i.e. supplied nodes could be 
		// descendants of the actualk siblings. 
		EQUAL,               // 0, equal 
		LEFT_IS_ANCESTOR,   // -1, l less than r
		RIGHT_IS_ANCESTOR,  // +1, l greater than r
		CONTAINER_SIBLINGS, // depends on child ptr order
		ITEM_SIBLINGS,      // depends on child ptr order 
		ROOT_SIBLINGS       // depends on child ptr order
	};
		
    DepthFirstRelation(const XTreeDatabase *db);

    /// Less operator: for use with set, map etc
    bool operator()( KeyType l_key, KeyType r_key ) const;
    Orderable::Diff Compare3Way( KeyType l_key, KeyType r_key ) const;
    pair<Orderable::Diff, RelType> CompareHierarchical( KeyType l_key, KeyType r_key ) const;

    void Test( const unordered_set<KeyType> &keys );
    
private:
	const XTreeDatabase * const db;
}; 

};

#endif
