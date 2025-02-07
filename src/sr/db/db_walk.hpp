#ifndef DB_WALK_HPP
#define DB_WALK_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_common.hpp"
#include "tree_zone.hpp"

namespace SR 
{
class DBWalk
{   
public:
    enum Context
    {
        ROOT,
        SINGULAR,
        IN_SEQUENCE,
        IN_COLLECTION
    };
    
    enum Wind
    {
        WIND_IN,
        WIND_OUT
    };

	// This structure is embedded inside our WalkInfo, and in LinkTable::Row.
	// We can fill it in successfully for all nodes only if walking from 
	// the true root of a tree. If we need to walk a subtree, we need to
	// be "booted" by receiving a copy if this info. We get that from the 
	// LinkTable.
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
		DBCommon::OrdinalType container_ordinal; 

		// Iterator on my_container that dereferences to me, if IN_SEQUENCE 
		// or IN_COLLECTION. Note: only used in regeneration queries.
        ContainerInterface::iterator container_it;
	};
	
    struct WalkInfo
    {
		CoreInfo core;              
		  		
        const TreePtrInterface *p_tree_ptr_interface;
        XLink xlink;
        TreePtr<Node> node;        
        const DBCommon::RootId *root_record;   
        bool at_terminus;     
        bool at_base;
	};

	typedef function<void (const WalkInfo &)> Action;
	typedef list< Action > Actions;
    
	void WalkTree( const Actions *actions,
				   XLink root_xlink,
				   const DBCommon::RootId *root_record, 
				   Wind wind );
    void WalkSubtree( const Actions *actions,
					  XLink base_xlink,
					  const DBCommon::RootId *root_record,
					  Wind wind,
					  const CoreInfo *base_info );
    void WalkZone( const Actions *actions,
                   TreeZone zone,
                   const DBCommon::RootId *root_record,
                   Wind wind,
                   const CoreInfo *base_info );
private:
    struct WalkKit
    {
        const Actions *actions;
        TreeZone zone;
		const DBCommon::RootId *root_record;        
		Wind wind;
		mutable TreeZone::TerminusIterator next_terminus_it;
    };

    void VisitBase( const WalkKit &kit, 
                    const CoreInfo *base_info );
    void VisitSingular( const WalkKit &kit, 
                        const TreePtrInterface *p_x_singular, 
                        XLink xlink,
                        int item_ordinal );
    void VisitSequence( const WalkKit &kit, 
                        SequenceInterface *x_seq, 
                        XLink xlink,
                        int item_ordinal );
    void VisitCollection( const WalkKit &kit, 
                          CollectionInterface *x_col, 
                          XLink xlink,
                          int item_ordinal );
    void VisitLink( const WalkKit &kit, 
                    WalkInfo &&walk_info );
    void VisitItemise( const WalkKit &kit, 
                       XLink xlink );
};    
    
}

#endif
