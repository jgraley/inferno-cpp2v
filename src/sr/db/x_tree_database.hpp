#ifndef X_TREE_DATABASE_HPP
#define X_TREE_DATABASE_HPP

#include "common/standard.hpp"
#include "zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "mutable_zone.hpp"

#include "link_table.hpp"
#include "node_table.hpp"
#include "domain.hpp"
#include "domain_extension.hpp"
#include "orderings.hpp"
#include "db_walk.hpp"
#include <queue>

namespace SR 
{    
      
class FreeZone;
class TreeZone;     
      
class XTreeDatabase : public Traceable
{
public:
    XTreeDatabase(shared_ptr<Lacing> lacing_, DomainExtension::ExtenderSet domain_extenders );
    
	// ---------------- the principal mutators ------------------
    
    // Zone goes into tree. Geom and intrinsic assets inserted. No deletes.
    DBCommon::TreeOrdinal BuildTree(DBCommon::TreeType tree_type, const FreeZone &free_zone);
    
    // Zone removed all assets deleted
    void TeardownTree(DBCommon::TreeOrdinal tree_ordinal);
   
	// Swap content between two different trees, performed on geom assets only
   	void SwapTreeToTree( TreeZone &tree_zone_l, vector<TreeZone *> fixups_l,
		 				 TreeZone &tree_zone_r, vector<TreeZone *> fixups_r );

	// Actions to take at the end of an update cycle
    void PerformDeferredActions();

	// ---------------- const and static methods ------------------
    XLink GetRootXLink(DBCommon::TreeOrdinal tree_ordinal) const;
    DBCommon::TreeType GetTreeType(DBCommon::TreeOrdinal tree_ordinal) const;

    const DomainExtensionChannel *GetDEChannel( const DomainExtension::Extender *extender ) const;
    const Domain &GetDomain() const;

    const LinkTable &GetLinkTable() const;
    const LinkTable::Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;    
    const NodeTable::Row &GetNodeRow(TreePtr<Node> node) const;
    bool HasNodeRow(TreePtr<Node> node) const;
    
    // Parent X link if not a root
    XLink TryGetParentXLink(XLink xlink) const;
    
    // XLink from TPI ptr
    XLink TryGetXLink( const TreePtrInterface *ptp ) const;
    XLink GetXLink( const TreePtrInterface *ptp ) const;
    
    // Last of the descendents in depth first order. If no 
    // descendents, it will be the supplied node. 
    static XLink GetLastDescendantXLink(XLink base);

    const Orderings &GetOrderings() const;
    
    TreePtr<Node> GetMainRootNode() const;
    XLink GetMainRootXLink() const;
	DBCommon::TreeOrdinal GetMainTreeOrdinal() const;
	DBCommon::TreeOrdinal GetTreeOrdinalFor(XLink xlink) const;

private: 
    Mutator CreateTreeMutator(XLink xlink) const;
    MutableZone CreateMutableZone(TreeZone &zone) const;

public:
    void Dump() const;
	void DumpTables() const;

    void CheckAssets();
    
private: 
    DBCommon::TreeOrdinal AllocateTree();
    
    void FreeTree(DBCommon::TreeOrdinal tree_ordinal);

    const shared_ptr<Lacing> lacing;
    const shared_ptr<Domain> domain;
    const shared_ptr<LinkTable> link_table;
    const shared_ptr<NodeTable> node_table;
    const shared_ptr<Orderings> orderings;
    const shared_ptr<DomainExtension> domain_extension;

    map<DBCommon::TreeOrdinal, DBCommon::TreeRecord> trees_by_ordinal;
    queue<DBCommon::TreeOrdinal> free_tree_ordinals;

    DBWalk db_walker;
    DBCommon::TreeOrdinal next_tree_ordinal;  
    DBCommon::TreeOrdinal main_tree_ordinal;  
};    
    
};

#endif
