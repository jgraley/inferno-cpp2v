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
	class Row : public DBWalk::CoreInfo,
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
		DBCommon::RootOrdinal root_ordinal = (DBCommon::RootOrdinal)(-1);      // TODO this is for eg X tree vs domain trees vs MMAX etc

		string GetTrace() const;
	};


    LinkTable();
    
    const Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;
    
    const DBWalk::CoreInfo &GetCoreInfo(XLink xlink) const;
    
    DBWalk::Action GetDeleteAction();
	DBWalk::Action GetInsertAction();
	
	void GenerateRow(const DBWalk::WalkInfo &walk_info);
	
	unique_ptr<Mutator> GetMutator(XLink xlink) const;

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
