#ifndef NODE_TABLE_HPP
#define NODE_TABLE_HPP

#include "../zone.hpp"
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

    class Row : public Traceable
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
		
        void Merge( const Row &nn );
        string GetTrace() const;
    };

    const Row &GetRow(TreePtr<Node> node) const;
    bool HasRow(TreePtr<Node> node) const;
    void ClearMonolithic();
    void Delete( const TreeZone &zone );

	void PopulateActions(DBWalk::Actions &actions);
	void PrepareBuildMonolithic(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);

private:
    // Node-to-row-of-x_tree_db map
    map<TreePtr<Node>, Row> rows;
};    
    
};

#endif
