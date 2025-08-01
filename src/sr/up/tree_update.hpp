#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"
#include "patches.hpp"

namespace SR 
{

// ------------------------- Runners --------------------------

class TreeUpdater
{
public:
    TreeUpdater(XTreeDatabase *x_tree_db); // db optional
    
	void BuildMainTree( TreePtr<Node> main_tree_root );
	void TeardownMainTree();

    // Duplicates all the tree zones and merges everything into one
    static unique_ptr<FreeZone> TransformToSingleFreeZone( shared_ptr<Patch> source_layout );
    
    // Much as the name suggests. Db required and acts directly on it.
    ReplaceAssignments UpdateMainTree( XLink origin_xlink, shared_ptr<Patch> source_layout );
    
private:
	void Analysis(XLink origin_xlink, shared_ptr<Patch> &source_layout, ReplaceAssignments &assignments);
	void ApplyUpdate(XLink origin_xlink, shared_ptr<Patch> &source_layout, ReplaceAssignments &assignments);

    XTreeDatabase * const db;
};    

}

#endif
