#ifndef LINK_TABLE_HPP
#define LINK_TABLE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"
#include "tree_zone.hpp"

namespace SR 
{
class NodeTable;    
    
class LinkTable : public Traceable
{
public:
    class Row : public DBCommon::CoreInfo,
                 public Traceable
    {
    public:
        // Looking for parent_node etc? Check DBWalk::CoreInfo which is a factored-out
        // core set of contextual info. All we need to do here is fill in other useful
        // fields that are derived from the core info.
                
        // First element of container of which I'm a member. 
        // Defined for all item types.
        XLink container_front = XLink();
        XLink container_back = XLink();

        // Neighbour elements within my sequence (sequences only)
        XLink sequence_predecessor = XLink();
        XLink sequence_successor = XLink();

        // Ordinals
        DBCommon::TreeOrdinal tree_ordinal = (DBCommon::TreeOrdinal)(-1);      // TODO this is for eg X tree vs domain trees vs MMAX etc

        string GetTrace() const;
    };

    LinkTable();
    
    const Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;
    
    const DBCommon::CoreInfo &GetCoreInfo(XLink xlink) const;
    
	void InsertTree(TreeZone &zone);
	void DeleteTree(TreeZone &zone);
    
	class RAIISuspendForSwap : RAIISuspendForSwapBase
	{
	public:
		RAIISuspendForSwap(LinkTable *link_table_, TreeZone &zone1_, TreeZone &zone2_ );
		~RAIISuspendForSwap();
	private:
		DBWalk db_walker;     
		LinkTable &link_table;
		unordered_map<XLink, Row> &rows; 
		queue<DBCommon::CoreInfo> terminus_info1;
		queue<DBCommon::CoreInfo> terminus_info2;
		DBCommon::CoreInfo mybase_info1;
		DBCommon::CoreInfo mybase_info2;
	};

	void InsertAction(const DBWalk::WalkInfo &walk_info);
	void DeleteAction(const DBWalk::WalkInfo &walk_info);
    
    void GenerateRow(XLink xlink, DBCommon::TreeOrdinal tree_ordinal, const DBCommon::CoreInfo *core_info);
    
    vector<XLink> GetXLinkDomainAsVector() const;
    
    string GetTrace() const;
    void Dump() const;
    
//private:
    DBWalk db_walker;     

    // XLink-to-row-of-x_tree_db map
    unordered_map<XLink, Row> rows; 
};    
    
};

#endif
