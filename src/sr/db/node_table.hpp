#ifndef NODE_TABLE_HPP
#define NODE_TABLE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"

namespace SR 
{    
    
class NodeTable : public Traceable
{
public:
    NodeTable();

    class Row : public Traceable
    {
    public:		
		// Our node is the child of these links.
		set<XLink> parents; // TODO use of the word "parent" is confusing: it's a link to us. "incoming"? 

		// Declarative XLinks onto our node. 
		// A subset of parents, so to get the declarer node, you'll need 
		// to use eg Row::TryGetParentXLink().GetChildX(). Why have I done 
		// this? So that this info is unambiguous across parallel links:
		// We'll uniquely specify the correct one if only one is a 
		// declaring link (precision). Taking parent discards that info.
		set<XLink> declarers;
		
        string GetTrace() const;
    };

    const Row &GetRow(TreePtr<Node> node) const;
    bool HasRow(TreePtr<Node> node) const;
    
    bool IsDeclarer(const DBWalk::WalkInfo &walk_info) const;
    
    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);

	string GetTrace() const;
private:
    // Node-to-row-of-x_tree_db map
    map<TreePtr<Node>, Row> rows;
};    
    
};

#endif
