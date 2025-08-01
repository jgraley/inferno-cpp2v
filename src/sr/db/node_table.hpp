#ifndef NODE_TABLE_HPP
#define NODE_TABLE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"
#include "tree_zone.hpp"

#include <unordered_map>

namespace SR 
{    
    
class LinkTable;

// Node table depends on link table so it can find parent node in order to 
// call GetDeclared() on it     
class NodeTable : public Traceable
{
public:
    NodeTable(const LinkTable *link_table_);

    class Row : public Traceable
    {
    public:        
        // Our node is the child of these links.
        set<XLink> incoming_xlinks; 

        // Declarative XLinks onto our node. 
        // A subset of incoming_xlinks, so to get the declarer node, you'll need 
        // to use eg Row::TryGetParentXLink().GetChildTreePtr(). Why have I done 
        // this? So that this info is unambiguous across parallel links:
        // We'll uniquely specify the correct one if only one is a 
        // declaring link (precision). Taking parent discards that info.
        set<XLink> declaring_xlinks;
        
        string GetTrace() const;
    };

    const Row &GetRow(TreePtr<Node> node) const;
    bool HasRow(TreePtr<Node> node) const;
    
    bool IsDeclarer(XLink xlink) const;
    
	void InsertTree(TreeZone &zone);
	void DeleteTree(TreeZone &zone);

	class SwapTransaction : DBCommon::SwapTransaction
	{
	public:
		SwapTransaction(NodeTable *node_table_, TreeZone &zone1_, TreeZone &zone2_ );
		~SwapTransaction();
	private:
		DBWalk db_walker;     
		NodeTable &node_table;
	};
		
	void InsertLink(XLink xlink);
    void DeleteLink(XLink xlink);

    vector<TreePtr<Node>> GetNodeDomainAsVector() const;

   	size_t GetTotNumNodes() const;
   	
   	void ConsistencyCheck(const XTreeDatabase *db) const;

    string GetTrace() const;
    void Dump() const;
    
private:
    DBWalk db_walker;  
    
    const LinkTable *link_table;

    // Node-to-row-of-x_tree_db map
    unordered_map<TreePtr<Node>, Row> rows;
};    
    
};

#endif
