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

//#define DB_ENABLE_COMPARATIVE_TEST

namespace SR 
{    
      
class XTreeDatabase : public Traceable, 
                      public Duplicate::DirtyGrassUpdateInterface
{
public:
    XTreeDatabase( XLink main_root_xlink, shared_ptr<Lacing> lacing, DomainExtension::ExtenderSet domain_extenders );
    
	// Use both monolithic and incremental updates in order 
	// to build full db during analysis stage
    void InitialBuild();

	// Incremental strategy: perform updates on zones
    void InsertMainTree(XLink xlink);
    void DeleteMainTree(XLink xlink);
    void InsertExtraTree(XLink xlink);
    void DeleteExtraTree(XLink xlink);

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

    bool IsDirtyGrass( TreePtr<Node> node ) const final;
    void AddDirtyGrass( TreePtr<Node> node ) const final;
    void ClearDirtyGrass();
    
    void Dump() const;
	
private:    
    void TestRelations();
    
private: 
    const shared_ptr<Domain> domain;
    const shared_ptr<NodeTable> node_table;
    const shared_ptr<LinkTable> link_table;
    const shared_ptr<Orderings> orderings;
    const shared_ptr<DomainExtension> domain_extension;

	map<XLink, DBCommon::RootRecord> roots;

  	XLink main_root_xlink;
    DBWalk db_walker;
    DBCommon::RootOrdinal next_root_ordinal;
    
    mutable set< TreePtr<Node> > dirty_grass; // See #724 re mutable
};    
    
};

#endif
