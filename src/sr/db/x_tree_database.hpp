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
class TreeZone;     
      
class XTreeDatabase : public Traceable
{
public:
    XTreeDatabase( TreePtr<Node> main_root, shared_ptr<Lacing> lacing, DomainExtension::ExtenderSet domain_extenders );
    
    // Update and access our trees. Some are created by the DB, others (the extra ones) are
    // allocated and freed here.
    DBCommon::TreeOrdinal AllocateExtraTree(TreePtr<Node> root_node);
    void FreeExtraTree(DBCommon::TreeOrdinal tree_ordinal);
    XLink GetRootXLink(DBCommon::TreeOrdinal tree_ordinal) const;
    
	// Use both monolithic and incremental updates in order 
	// to build full db during analysis stage
    void InitialBuild();
    
	// Incremental strategy: perform updates on zones
    void MainTreeReplace( TreeZone target_tree_zone, FreeZone source_free_zone );
		
    void MainTreeDelete(TreeZone zone, const DBCommon::CoreInfo *base_info);
    void MainTreeInsert(TreeZone zone, const DBCommon::CoreInfo *base_info);

	void PerformQueuedExtraTreeActions();
    void ExtraTreeDelete(DBCommon::TreeOrdinal tree_ordinal);
    void ExtraTreeInsert(DBCommon::TreeOrdinal tree_ordinal);

    const DomainExtensionChannel *GetDEChannel( const DomainExtension::Extender *extender ) const;
    void PostUpdateActions();
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
	XLink GetLastDescendant(XLink xlink) const;

	const Orderings &GetOrderings() const;
	
	TreePtr<Node> GetMainRootNode() const;
	XLink GetMainRootXLink() const;

   	unique_ptr<Mutator> GetMutator(XLink xlink);

    void Dump() const;
	
private:    
    void TestRelations();
    
private: 
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
};    
    
};

#endif
