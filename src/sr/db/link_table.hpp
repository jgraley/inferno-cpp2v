#ifndef LINK_TABLE_HPP
#define LINK_TABLE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"

namespace SR 
{
class NodeTable;    
    
// So I can forward-declare it TODO factor out common minimal LinkContext and put in DBWalk
class LinkTableRow : public Traceable
{
public:
	string GetTrace() const;
	
	TreePtr<Node> parent_node;
	
	// Index into itemisation of the parent node
	int item_ordinal;

	DBWalk::Context containment_context;

	ContainerInterface *p_xcon;

	// First element of container of which I'm a member. 
	// Defined for all item types.
	XLink my_container_front = XLink();
	XLink my_container_back = XLink();

	// Iterator on my_container that dereferneces to me, if 
	// IN_SEQUENCE or IN_COLLECTION. Note: only used in regeneration
	// queries.
	ContainerInterface::iterator my_container_it_predecessor;
	ContainerInterface::iterator my_container_it;

	// Neighbour elements within my sequence (sequences only)
	XLink my_sequence_predecessor = XLink();
	XLink my_sequence_successor = XLink();

	// Ordinals
	DBCommon::OrdinalType container_ordinal = -1; 
	DBCommon::RootOrdinal root_ordinal = (DBCommon::RootOrdinal)(-1);      // TODO this is for eg X tree vs domain trees vs MMAX etc
};


class LinkTable : public Traceable
{
public:
	typedef LinkTableRow Row;

    LinkTable();
    
    const Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;
    
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
