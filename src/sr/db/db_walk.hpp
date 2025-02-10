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
    enum Wind
    {
        WIND_IN,
        WIND_OUT
    };
	
    struct WalkInfo
    {
		DBCommon::CoreInfo core;              
		  		
        const TreePtrInterface *p_tree_ptr_interface;
        XLink xlink;
        TreePtr<Node> node;        
        const DBCommon::RootId root_id;   // TODO surely in the core info as const across walk?
        bool at_terminus;     
        bool at_base;
	};

	typedef function<void (const WalkInfo &)> Action;
	typedef list< Action > Actions;
    
	void WalkTree( const Actions *actions,
				   XLink root_xlink,
				   const DBCommon::RootId root_id, 
				   Wind wind );
    void WalkSubtree( const Actions *actions,
					  XLink base_xlink,
					  const DBCommon::RootId root_id,
					  Wind wind,
					  const DBCommon::CoreInfo *base_info );
    void WalkZone( const Actions *actions,
                   TreeZone zone,
                   const DBCommon::RootId root_id,
                   Wind wind,
                   const DBCommon::CoreInfo *base_info );
private:
    struct WalkKit
    {
        const Actions *actions;
        const TreeZone &zone;
		const DBCommon::RootId root_id;        
		Wind wind;
		mutable TreeZone::TerminusIterator next_terminus_it;
    };

    void VisitBase( const WalkKit &kit, 
                    const DBCommon::CoreInfo *base_info );
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
