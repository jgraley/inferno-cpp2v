#ifndef DB_WALK_HPP
#define DB_WALK_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_common.hpp"
#include "tree_zone.hpp"

namespace SR 
{
class LinkTableRow;

class DBWalk
{   
public:
    enum Context
    {
        ROOT,
        LEGACY_BASE,
        SINGULAR,
        IN_SEQUENCE,
        IN_COLLECTION
    };
    
    enum Wind
    {
        WIND_IN,
        WIND_OUT
    };

    struct CoreInfo
    {
		TreePtr<Node> parent_node;
		
		// Index into itemisation of the parent node
		int item_ordinal;
		
		// What type of container is the incoming link?
		Context context_type;
		
		// Pointer to the container (if really a container)
		ContainerInterface *p_container;
		
		// Index inside container (if really a container)
		DBCommon::OrdinalType container_ordinal; 

		// Iterator on my_container that dereferences to me, if 
		// IN_SEQUENCE or IN_COLLECTION. Note: only used in regeneration
		// queries.
        ContainerInterface::iterator container_it;
	};
	
    struct WalkInfo
    {
		CoreInfo core;                		
        const TreePtrInterface *p_x;
        XLink xlink;
        TreePtr<Node> x;
        
        const DBCommon::RootRecord *root_record;   
        bool at_terminus;     
        bool at_base;
	};

	typedef function<void (const WalkInfo &)> Action;
	typedef list< Action > Actions;
    
    void Walk( const Actions *actions,
               XLink base_xlink,
               const DBCommon::RootRecord *root_record,
               Wind wind,
               const LinkTableRow *base_link_row );
    void Walk( const Actions *actions,
               TreeZone zone,
               const DBCommon::RootRecord *root_record,
               Wind wind,
               const LinkTableRow *base_link_row );
private:
    struct WalkKit
    {
        const Actions *actions;
        TreeZone zone;
		const DBCommon::RootRecord *root_record;        
		Wind wind;
		mutable TreeZone::TerminusIterator next_terminus_it;
    };

    void VisitBase( const WalkKit &kit, 
                    const LinkTableRow *base_link_row );
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
