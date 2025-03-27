#ifndef LINK_TABLE_HPP
#define LINK_TABLE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"

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
    
    DBWalk::Action GetDeleteGeometricAction();
    DBWalk::Action GetInsertGeometricAction();
    
    void GenerateRow(const DBWalk::WalkInfo &walk_info);
    
    vector<XLink> GetXLinkDomainAsVector() const;
    
    string GetTrace() const;
    
//private:
    typedef XLink Key;

    // XLink-to-row-of-x_tree_db map
    unordered_map<Key, Row> rows; 
};    
    
};

#endif
