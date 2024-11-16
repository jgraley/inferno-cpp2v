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
        BASE,
        SINGULAR,
        IN_SEQUENCE,
        IN_COLLECTION
    };
    
    enum Wind
    {
        WIND_IN,
        WIND_OUT
    };

    struct WalkInfo
    {
        TreePtr<Node> parent_x;
        int item_ordinal;
		Context context;
        ContainerInterface *p_xcon;
        int container_ordinal;
        ContainerInterface::iterator xit_predecessor;
        ContainerInterface::iterator xit;
        const TreePtrInterface *p_x;
        XLink xlink;
        TreePtr<Node> x;
        const DBCommon::RootRecord *root_record;   
        bool terminus;     
	};

	typedef function<void (const WalkInfo &)> Action;
	typedef list< Action > Actions;
    
    void Walk( const Actions *actions,
               XLink base_xlink,
               Context base_context,
               const DBCommon::RootRecord *root_record,
               Wind wind );
    void Walk( const Actions *actions,
               TreeZone zone,
               Context base_context,
               const DBCommon::RootRecord *root_record,
               Wind wind );
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
                    Context context );
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
