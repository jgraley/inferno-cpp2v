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
        SINGULAR,
        IN_SEQUENCE,
        IN_COLLECTION
    };
    
    struct WalkInfo
    {
        TreePtr<Node> parent_x;
        int item_ordinal;
		ContainmentContext context;
        ContainerInterface *p_xcon;
        int container_ordinal;
        ContainerInterface::iterator xit_predecessor;
        ContainerInterface::iterator xit;
        const TreePtrInterface *p_x;
        XLink xlink;
        TreePtr<Node> x;
	};

	struct Actions
	{
		function<void (const WalkInfo &)> domain_in;
		function<void (const WalkInfo &)> domain_out;

		function<void (const WalkInfo &)> domain_extension_in;
		function<void (const WalkInfo &)> domain_extension_out;

		function<void (const WalkInfo &)> link_row_in;
		function<void (const WalkInfo &)> link_row_out;

		function<void (const WalkInfo &)> node_row_in;
		function<void (const WalkInfo &)> node_row_out;

		function<void (const WalkInfo &)> indexes_in;
		function<void (const WalkInfo &)> indexes_out;
	};
    
    void Walk( const Actions *actions,
               XLink base_xlink,
               ContainmentContext base_context );
private:
    struct WalkKit
    {
        const Actions *actions;
        XLink base_xlink;
    };

    void VisitBase( const WalkKit &kit, 
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
                       XLink xlink,
                       TreePtr<Node> x );

    void WindInActions( const WalkKit &kit, 
                        const WalkInfo &walk_info );
    void UnwindActions( const WalkKit &kit, 
                        const WalkInfo &walk_info );
};    
    
}

#endif
