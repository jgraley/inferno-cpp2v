#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// ------------------------- Runners --------------------------

class TreeUpdater
{
public:
    TreeUpdater(XTreeDatabase *x_tree_db = nullptr); // db optional
    
	void BuildMainTree( TreePtr<Node> main_tree_root );
	void TeardownMainTree();

    // Duplicates all the tree zones and merges everything into one
    unique_ptr<FreeZone> TransformToSingleFreeZone( shared_ptr<Patch> source_layout );
    
    // Much as the name suggests. Db required and acts directly on it.
    void UpdateMainTree( XLink origin_xlink, shared_ptr<Patch> source_layout );
    
private:
    XTreeDatabase * const db;
};    

}

#endif
