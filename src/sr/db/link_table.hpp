#ifndef LINK_TABLE_HPP
#define LINK_TABLE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"

namespace SR 
{
class NodeTable;    
    
// So I can forward-declare it TODO factor out common minimal LinkContext and put in DBWalk
class LinkTableRow : public DBWalk::CoreInfo,
					 public Traceable
{
public:
	// Looking for parent_node etc? Check DBWalk::CoreInfo which is a factored-out
	// core set of contextual info.
			
	// First element of container of which I'm a member. 
	// Defined for all item types.
	XLink container_front = XLink();
	XLink container_back = XLink();

	// Neighbour elements within my sequence (sequences only)
	XLink sequence_predecessor = XLink();
	XLink sequence_successor = XLink();

	// Ordinals
	DBCommon::RootOrdinal root_ordinal = (DBCommon::RootOrdinal)(-1);      // TODO this is for eg X tree vs domain trees vs MMAX etc

	string GetTrace() const;
};


class LinkTable : public Traceable
{
public:
	typedef LinkTableRow Row;

    LinkTable();
    
    const Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;
    
    const DBWalk::CoreInfo &GetCoreInfo(XLink xlink) const;
    
    DBWalk::Action GetDeleteAction();
	DBWalk::Action GetInsertAction();
	
	void GenerateRow(const DBWalk::WalkInfo &walk_info);
	
	string GetTrace() const;

//private:
	typedef const TreePtrInterface * Key;

    // XLink-to-row-of-x_tree_db map
    unordered_map<Key, Row> rows;
    
    // Base ordering
    int current_root_ordinal;    
};    
    
};

#endif
