#ifndef TABLES_HPP
#define TABLES_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"

namespace SR 
{
class SimpleCompareQuotientSet;
class VNStep;
class Lacing;
    
class Tables
{
public:
    Tables();

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
    
    class NodeRow : public Traceable
    {
    public:		
		// Our node is the child of these links.
		set<XLink> parents;

		// Declarative XLinks onto our node. 
		// A subset of parents, so to get the declarer node, you'll need 
		// to use eg Row::parent_xlink.GetChildX(). Why have I done 
		// this? So that this info is unambiguous across parallel links:
		// We'll uniquely specify the correct one if only one is a 
		// declaring link (precision). Taking parent discards that info.
		set<XLink> declarers;
		
        void Merge( const NodeRow &nn );
        string GetTrace() const;
    };

    const Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;
    
    const NodeRow &GetNodeRow(TreePtr<Node> node) const;
    bool HasNodeRow(TreePtr<Node> node) const;

	void PopulateActions(DBWalk::Actions &actions);
	void PrepareFullBuild(DBWalk::Actions &actions);
	void PrepareExtraXLink(DBWalk::Actions &actions);

    // XLink-to-row-of-x_tree_db map
    unordered_map<XLink, Row> xlink_table;

    // Node-to-row-of-x_tree_db map
    map<TreePtr<Node>, NodeRow> node_table;
    
private:
    // Depth-first ordering
    int current_ordinal;
    
    // Last node to be reached and given a row
    XLink last_xlink;
    
    DBWalk db_walker;
};    
    
};

#endif
