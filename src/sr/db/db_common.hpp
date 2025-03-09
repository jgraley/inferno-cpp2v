#ifndef DB_COMMON_HPP
#define DB_COMMON_HPP

#include "../link.hpp"
#include "common/standard.hpp"

#define CAT_BY_NODE


namespace SR 
{
class DBCommon
{
public:  
    typedef int OrdinalType; // other ordinals...

	enum class TreeOrdinal
	{
		MAIN,
		EXTRAS
		// ...continues, EXTRAS+i
	};
    
    struct TreeRecord
    {
		shared_ptr<TreePtr<Node>> sp_tp_root_node;
	};
    
    enum Context
    {
        ROOT,
        SINGULAR,
        IN_SEQUENCE,
        IN_COLLECTION
    };
    
   	// This structure is embedded inside our DBWalk::WalkInfo, and in LinkTable::Row.
	// We can fill it in successfully for all nodes only if walking from 
	// the true root of a tree. If we need to walk a subtree, we need to
	// be "booted" by receiving a copy if this info. We get that from the 
	// LinkTable. They originate in DBWalk::Walk() from a tree root.
    struct CoreInfo
    {
		TreePtr<Node> parent_node;
		
		// Index into itemisation of the parent node
		int item_ordinal;
		
		// What type of container is the incoming link?
		Context context_type;
		
		// Pointer to the container (if in a container)
		ContainerInterface *p_container;
		
		// Index inside container (if in a container)
		OrdinalType container_ordinal; 

		// Iterator on my_container that dereferences to me, if IN_SEQUENCE 
		// or IN_COLLECTION. Note: only used in regeneration queries.
        ContainerInterface::iterator container_it;
	};
};    

}

#endif
