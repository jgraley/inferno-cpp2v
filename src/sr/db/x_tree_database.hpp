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
    XTreeDatabase( XLink main_root_xlink, shared_ptr<Lacing> lacing, DomainExtension::ExtenderSet domain_extenders );
    
	// Use both monolithic and incremental updates in order 
	// to build full db during analysis stage
    void InitialBuild();
    
	// Incremental strategy: perform updates on zones
    void MainTreeReplace( TreeZone target_tree_zone, FreeZone source_free_zone );
		
    void MainTreeDelete(TreeZone zone, const DBWalk::CoreInfo *base_info);
    void MainTreeInsert(TreeZone zone, const DBWalk::CoreInfo *base_info);

    void ExtraTreeDelete(XLink xlink);
    void ExtraTreeInsert(XLink xlink);

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
    
    void Dump() const;
	
private:    
    void TestRelations();
    
private: 
    const shared_ptr<Domain> domain;
    const shared_ptr<LinkTable> link_table;
    const shared_ptr<NodeTable> node_table;
    const shared_ptr<Orderings> orderings;
    const shared_ptr<DomainExtension> domain_extension;

	map<XLink, DBCommon::RootId> roots;

  	XLink main_root_xlink;
    DBWalk db_walker;
    DBCommon::RootOrdinal next_root_ordinal;
        
    queue<XLink> de_extra_insert_queue;
    queue<XLink> de_extra_delete_queue;
};    
    
};

#endif
