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
                      public NavigationUtils,
                      public Duplicate::DirtyGrassUpdateInterface
{
public:
    XTreeDatabase( XLink root_xlink, shared_ptr<Lacing> lacing, DomainExtension::ExtenderSet domain_extenders );
    
private: 
    const struct Plan : public Traceable
    {
        Plan( const XTreeDatabase *algo, shared_ptr<Lacing> lacing, DomainExtension::ExtenderSet domain_extenders );
        
        shared_ptr<Domain> domain;
        shared_ptr<NodeTable> node_table;
        shared_ptr<LinkTable> link_table;
        shared_ptr<Orderings> indexes;
        shared_ptr<DomainExtension> domain_extension;
    } plan;

public:
	// Use both monolithic and incremental updates in order 
	// to build full db during analysis stage
    void InitialBuild();

	// Incremental strategy: perform updates on zones
    void Delete(XLink base);
    void Insert(XLink base);
    void InsertExtraZone(XLink extra_base);
    void DeleteExtraZone(XLink extra_base);

    void InitialWalk( const DBWalk::Actions *actions,
                      XLink root_xlink );

    const DomainExtensionChannel *GetDEChannel( const DomainExtension::Extender *extender ) const;
    void CompleteDomainExtension();
   	const Domain &GetDomain() const;

    const LinkTable &GetLinkTable() const;
    const LinkTable::Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;    
    const NodeTable::Row &GetNodeRow(TreePtr<Node> node) const;
    bool HasNodeRow(TreePtr<Node> node) const;
    
    // Parent X link if not a base
	XLink TryGetParentXLink(XLink xlink) const;
    
    // Last of the descendents in depth first order. If no 
    // descendents, it will be the supplied node. 
	XLink GetLastDescendant(XLink xlink) const;

	const Orderings &GetOrderings() const;
	TreePtr<Node> GetRootNode() const;
	XLink GetRootXLink() const;

    bool IsDirtyGrass( TreePtr<Node> node ) const final;
    void AddDirtyGrass( TreePtr<Node> node ) const final;
    void ClearDirtyGrass();
    
    void Dump() const;
	
private:    
    // TreeKit implementation
    bool IsRequireReports() const override;
   
  	set<LinkInfo> GetParents( TreePtr<Node> node ) const override;
  	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
  	
    void TestRelations();
    
  	XLink root_xlink;
    DBWalk db_walker;
    
    mutable set< TreePtr<Node> > dirty_grass; // See #724 re mutable
};    
    
};

#endif
