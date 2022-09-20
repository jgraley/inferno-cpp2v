#ifndef DB_WALK_HPP
#define DB_WALK_HPP

#include "../link.hpp"
#include "common/standard.hpp"

namespace SR 
{

class DBWalk
{   
public:
    enum ContainmentContext
    {
        ROOT,
        SINGULAR,
        IN_SEQUENCE,
        IN_COLLECTION
    };
    
    struct WalkInfo
    {
		ContainmentContext context;
        XLink xlink;
        XLink parent_xlink;
        ContainerInterface *p_xcon;
        ContainerInterface::iterator xit_predecessor;
        ContainerInterface::iterator xit;
        const TreePtrInterface *p_x;
	};

	struct Actions
	{
		function<bool (const WalkInfo &)> domain_in_is_ok;
		function<void (const WalkInfo &)> domain_in;
		function<list<XLink>::const_iterator(const WalkInfo &)> indexes_in;
		function<void (const WalkInfo &, list<XLink>::const_iterator)> xlink_row_in;
		function<void (const WalkInfo &)> node_row_in;
		function<void (const WalkInfo &)> xlink_row_out;
	};

	void FullWalk( const Actions &actions, 
				   XLink root_xlink );
	void ExtraXLinkWalk( const Actions &actions, 
	                     XLink extra_xlink );

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
        STOP_IF_ALREADY_IN
    };

    void AddAtRoot( const Actions &actions, 
                    SubtreeMode mode,  
                    XLink root_xlink );
    void AddSingularNode( const Actions &actions, 
                          SubtreeMode mode, 
                          const TreePtrInterface *p_x_singular, 
                          XLink xlink );
    void AddSequence( const Actions &actions, 
                      SubtreeMode mode, 
                      SequenceInterface *x_seq, 
                      XLink xlink );
    void AddCollection( const Actions &actions, 
                        SubtreeMode mode, 
                        CollectionInterface *x_col, 
                        XLink xlink );
    void AddLink( const Actions &actions, 
                  SubtreeMode mode, 
                  const WalkInfo &walk_info );
    void AddChildren( const Actions &actions, 
                      SubtreeMode mode, 
                      XLink xlink );
};    
    
}

#endif
