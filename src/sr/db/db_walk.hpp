#ifndef DB_WALK_HPP
#define DB_WALK_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_common.hpp"
#include "tree_zone.hpp"
#include "free_zone.hpp"

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
        const DBCommon::TreeOrdinal tree_ordinal;   // TODO surely in the core info as const across walk?
        bool at_terminus;     
        bool at_base;
    };

    typedef function<void (const WalkInfo &)> Action;
    typedef list< Action > Actions;
   
    void WalkTreeZone( const Actions *actions,
                       const TreeZone *zone,
                       const DBCommon::TreeOrdinal tree_ordinal,
                       Wind wind,
                       const DBCommon::CoreInfo *base_info );
    void WalkFreeZone( const Actions *actions,
                       const FreeZone *zone,
                       Wind wind );

private:
    struct WalkKit
    {
        const Actions *actions;
        const TreeZone *tree_zone;
        const DBCommon::TreeOrdinal tree_ordinal;        
        Wind wind;
        mutable size_t next_terminus_index;
    };

    void VisitBase( const WalkKit &kit, 
					XLink base_xlink,
                    TreePtr<Node> base_node,
                    const DBCommon::CoreInfo *base_info );
    void VisitSingular( const WalkKit &kit, 
                        const TreePtrInterface *p_x_singular, 
                        TreePtr<Node> node,
                        int item_ordinal );
    void VisitSequence( const WalkKit &kit, 
                        SequenceInterface *x_seq, 
                        TreePtr<Node> node,
                        int item_ordinal );
    void VisitCollection( const WalkKit &kit, 
                          CollectionInterface *x_col, 
                          TreePtr<Node> node,
                          int item_ordinal );
    void VisitNode( const WalkKit &kit, 
                    WalkInfo &&walk_info );
    void VisitItemise( const WalkKit &kit, 
                       TreePtr<Node> node );
};    
    
}

#endif
