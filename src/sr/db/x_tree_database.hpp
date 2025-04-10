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

// Drop -ui arg

namespace SR 
{    
      
class FreeZone;
class XTreeZone;     
      
class XTreeDatabase : public Traceable
{
public:
    XTreeDatabase( TreePtr<Node> main_root, shared_ptr<Lacing> lacing_, DomainExtension::ExtenderSet domain_extenders );
    
    // Update and access our trees. Some are created by the DB, others (the extra ones) are
    // allocated and freed here.
    DBCommon::TreeOrdinal AllocateExtraTree(TreePtr<Node> root_node);
    void FreeExtraTree(DBCommon::TreeOrdinal tree_ordinal);
    XLink GetRootXLink(DBCommon::TreeOrdinal tree_ordinal) const;
    vector<XLink> GetExtraRootXLinks() const;
    
    void WalkAllTrees(const DBWalk::Actions *actions,
                       DBWalk::Wind wind);
                                     
    // Use both monolithic and incremental updates in order 
    // to build full db during analysis stage
    void InitialBuild();
    
    // Incremental strategy: perform updates on zones
    void MainTreeExchange( MutableTreeZone *target_tree_zone, FreeZone *free_zone );
        
    void MainTreeDeleteGeometric(TreeZone *zone, const DBCommon::CoreInfo *base_info);
    void MainTreeInsertGeometric(TreeZone *zone, const DBCommon::CoreInfo *base_info);

    void MainTreeDeleteIntrinsic(TreeZone *zone);
    void MainTreeInsertIntrinsic(FreeZone *zone);

    void PerformQueuedExtraTreeActions();
    void ExtraTreeDelete(DBCommon::TreeOrdinal tree_ordinal);
    void ExtraTreeInsert(DBCommon::TreeOrdinal tree_ordinal);

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
    shared_ptr<Mutator> GetLastDescendantMutator(shared_ptr<Mutator> base);

    const Orderings &GetOrderings() const;
    
    TreePtr<Node> GetMainRootNode() const;
    XLink GetMainRootXLink() const;
	shared_ptr<Mutator> GetMainRootMutator();

    shared_ptr<Mutator> GetTreeMutator(XLink xlink);
    unique_ptr<MutableTreeZone> MakeMutableTreeZone(XLink xlink,
                                                    vector<XLink> terminii);

	void ClearMutatorCache();

    void Dump() const;
    
private:    
    void Checks();
    
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
        
    queue<DBCommon::TreeOrdinal> de_extra_insert_queue;
    queue<DBCommon::TreeOrdinal> extra_tree_destroy_queue;
    
    // Mutator cache holds shared pointers but is really a set of
    // Mutators, i.e. no two equal mutators allowed even though they
    // are separate objects and so the shared_ptrs differ.
    typedef DereferencingHash<shared_ptr<Mutator>, Mutator> SPMHasher;
    typedef DereferencingEqual<shared_ptr<Mutator>> SPMEqualPred;
    
   	unordered_set<shared_ptr<Mutator>, SPMHasher, SPMEqualPred> mutator_cache;
};    
    
};

#endif
