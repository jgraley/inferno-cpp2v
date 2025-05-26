#ifndef X_TREE_DATABASE_HPP
#define X_TREE_DATABASE_HPP

#include "common/standard.hpp"
#include "zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"

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
class XTreeZone;     
      
class XTreeDatabase : public Traceable
{
public:
    XTreeDatabase(shared_ptr<Lacing> lacing_, DomainExtension::ExtenderSet domain_extenders );
    
    // Update and access our trees. Some are created by the DB, others (the extra ones) are
    // allocated and freed here.
    DBCommon::TreeOrdinal AllocateExtraTree();
    void FreeExtraTree(DBCommon::TreeOrdinal tree_ordinal);
    XLink GetRootXLink(DBCommon::TreeOrdinal tree_ordinal) const;
    vector<XLink> GetExtraRootXLinks() const;
    
    void WalkAllTrees(const DBWalk::Actions *actions,
                       DBWalk::Wind wind);
                                     
    // Zone goes into tree. Geom and intrinsic assets inserted. No deletes.
    MutableTreeZone BuildTree(DBCommon::TreeOrdinal tree_ordinal, const FreeZone &free_zone);
    
    // Zone removed all assets deleted
    void TeardownTree(DBCommon::TreeOrdinal tree_ordinal);
   
	// Swap content between two different trees, performed on geom assets only
   	void SwapTreeToTree( DBCommon::TreeOrdinal tree_ordinal_l, MutableTreeZone &tree_zone_l, vector<MutableTreeZone *> fixups_l,
		 				 DBCommon::TreeOrdinal tree_ordinal_r, MutableTreeZone &tree_zone_r, vector<MutableTreeZone *> fixups_r );

    void PerformDeferredActions();

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

    Mutator CreateTreeMutator(XLink xlink);
    MutableTreeZone CreateMutableTreeZone(XLink xlink,
                                          vector<XLink> terminii);

    void Dump() const;
	void DumpTables() const;

    void CheckAssets();
    
private: 
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
};    
    
};

#endif
