#ifndef LINK_TABLE_HPP
#define LINK_TABLE_HPP

#include "../zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"

namespace SR 
{
    
class LinkTable : public Traceable
{
public:
    LinkTable();

    class Row : public Traceable
    {
    public:
        DBWalk::ContainmentContext containment_context;
        
        // Parent X link if not ROOT
        // Note that the parent is unique because:
        // - row is relative to a link, not a node,
        // - multiple parents only allowed at leaf, and parent is 
        //   (at least) one level back from that.
        XLink parent_xlink = XLink();
        
        // Last of the descendents in depth first order. If no 
        // descendents, it will be the current node. 
        XLink last_descendant_xlink = XLink();
        
        // First element of container of which I'm a member. 
        // Defined for all item types.
        XLink my_container_front = XLink();
        XLink my_container_back = XLink();

        // Neighbour elements within my sequence (sequences only)
        XLink my_sequence_predecessor = XLink();
        XLink my_sequence_successor = XLink();

        // Index in a depth-first walk
        DBCommon::OrdinalType depth_first_ordinal = -1;
        
        // Iterator in a depth-first walk
        DBCommon::DepthFirstOrderedIt depth_first_ordered_it;
        
        // Iterator on my_container that dereferneces to me, if 
        // IN_SEQUENCE or IN_COLLECTION. Note: only used in regeneration
        // queries.
        ContainerInterface::iterator my_container_it;
               
        string GetTrace() const;
    };
    
    const Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;
    
    void MonolithicClear();
	void PrepareMonolithicBuild(DBWalk::Actions &actions);
    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);
	
	string GetTrace() const;

private:
    // XLink-to-row-of-x_tree_db map
    unordered_map<XLink, Row> rows;

    // Depth-first ordering
    int current_ordinal;
    
    // Last node to be reached and given a row
    XLink last_xlink;
};    
    
};

#endif
