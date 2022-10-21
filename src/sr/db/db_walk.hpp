#ifndef DB_WALK_HPP
#define DB_WALK_HPP

#include "../zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "db_common.hpp"

namespace SR 
{

class DBWalk
{   
public:
    enum ContainmentContext
    {
        UNKNOWN,
        ROOT,
        EXTRA,
        SPECIAL,
        SINGULAR,
        IN_SEQUENCE,
        IN_COLLECTION
    };
    
    struct WalkInfo
    {
		ContainmentContext context;
        XLink xlink;
        XLink parent_xlink;
        int item_ordinal;
        int container_ordinal;
        const TreePtrInterface *p_x;
        ContainerInterface *p_xcon;
        ContainerInterface::iterator xit_predecessor;
        ContainerInterface::iterator xit;
	};

	struct Actions
	{
		function<bool (const WalkInfo &)> is_unreached;
		function<void (const WalkInfo &)> domain_in;
		function<void (const WalkInfo &)> indexes_in;
		function<void (const WalkInfo &)> link_row_in;
		function<void (const WalkInfo &)> indexes_in_late;
		function<void (const WalkInfo &)> node_row_in;
		function<void (const WalkInfo &)> link_row_out;
	};
    
	void FullWalk( const Actions *actions, 
				   XLink root_xlink );
    void InitWalk( const Actions *actions );
    void ZoneWalk( const Actions *actions,
                   const TreeZone &zone );
    void ExtraZoneWalk( const Actions *actions, 
	                    XLink extra_base_xlink );
    void SingleXLinkWalk( const Actions *actions, 
	                      XLink xlink );

private:
    enum SubtreeMode
    {
        // Behaviour for main domain population: we will check uniqueness
        // of the XLinks we meet during our recursive walk.
        REQUIRE_SOLO,
        
        // Behaviour for domain extensions. We will continue as long as 
        // nodes are not already in the domain. If a node is in the 
        // domain, we don't recurse into it since everything under it
        // will also be in the domain. 
        // https://github.com/jgraley/inferno-cpp2v/issues/213#issuecomment-728266001
        STOP_IF_ALREADY_IN,
        
        // Only visit the base, for when we have an explicit list of xtrees and 
        // are looping though that.
        NO_RECURSE
    };

    struct WalkKit
    {
        const Actions *actions;
        SubtreeMode mode;
    };

    void VisitBase( const WalkKit &kit, 
                    XLink root_xlink,
                    ContainmentContext context );
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
                    const WalkInfo &walk_info );
    void VisitItemise( const WalkKit &kit, 
                       XLink xlink );

    void WindInActions( const WalkKit &kit, 
                        const WalkInfo &walk_info );
    void UnwindActions( const WalkKit &kit, 
                        const WalkInfo &walk_info );
};    
    
}

#endif
