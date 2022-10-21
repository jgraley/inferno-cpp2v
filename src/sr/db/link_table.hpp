#ifndef LINK_TABLE_HPP
#define LINK_TABLE_HPP

#include "../zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"

namespace SR 
{
class NodeTable;    
    
class LinkTable : public Traceable
{
public:
    LinkTable();

    class Row : public Traceable
    {
    public:
        string GetTrace() const;

        DBWalk::ContainmentContext containment_context;
        
        TreePtr<Node> parent_node;
        
        // Index into itemisation of the parent node
        int item_ordinal;

        // First element of container of which I'm a member. 
        // Defined for all item types.
        XLink my_container_front = XLink();
        XLink my_container_back = XLink();

        // Iterator on my_container that dereferneces to me, if 
        // IN_SEQUENCE or IN_COLLECTION. Note: only used in regeneration
        // queries.
        ContainerInterface::iterator my_container_it;

        // Neighbour elements within my sequence (sequences only)
        XLink my_sequence_predecessor = XLink();
        XLink my_sequence_successor = XLink();

        // Ordinals
        DBCommon::OrdinalType container_ordinal = -1; 
        DBCommon::OrdinalType base_ordinal = -1;                
    };
    
    const Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;
    
    void MonolithicClear();
	void PrepareMonolithicBuild(DBWalk::Actions &actions);
    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);
	
	string GetTrace() const;

private:
	typedef const TreePtrInterface * Key;

    // XLink-to-row-of-x_tree_db map
    unordered_map<Key, Row> rows;
    
    // Base ordering
    int current_base_ordinal;    
};    
    
};

#endif
