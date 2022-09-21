#ifndef NODE_TABLE_HPP
#define NODE_TABLE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"

namespace SR 
{
class SimpleCompareQuotientSet;
class VNStep;
class Lacing;
    
class NodeTable
{
public:
    NodeTable();

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

    const NodeRow &GetNodeRow(TreePtr<Node> node) const;
    bool HasNodeRow(TreePtr<Node> node) const;

	void PopulateActions(DBWalk::Actions &actions);
	void PrepareFullBuild(DBWalk::Actions &actions);
	void PrepareExtraXLink(DBWalk::Actions &actions);

    // Node-to-row-of-x_tree_db map
    map<TreePtr<Node>, NodeRow> node_table;
    
private:
};    
    
};

#endif
