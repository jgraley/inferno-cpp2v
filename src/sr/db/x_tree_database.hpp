#ifndef X_TREE_DATABASE_HPP
#define X_TREE_DATABASE_HPP

#include "common/standard.hpp"
#include "../zone.hpp"
#include "../link.hpp"

#include "link_table.hpp"
#include "node_table.hpp"
#include "domain.hpp"
#include "indexes.hpp"
#include "db_walk.hpp"

//#define DB_ENABLE_COMPARATIVE_TEST

namespace SR 
{    
    
class XTreeDatabase : public Traceable, 
                      public TreeKit
{
public:
    XTreeDatabase( shared_ptr<Lacing> lacing, XLink root_xlink );
    
private: 
    const struct Plan : public Traceable
    {
        Plan( const XTreeDatabase *algo, shared_ptr<Lacing> lacing );
        
        shared_ptr<Domain> domain;
        shared_ptr<NodeTable> node_table;
        shared_ptr<LinkTable> link_table;
        shared_ptr<Indexes> indexes;

#ifdef DB_ENABLE_COMPARATIVE_TEST
        shared_ptr<Domain> ref_domain;
        shared_ptr<Indexes> ref_indexes;
        //shared_ptr<NodeTable> ref_node_table;
        //shared_ptr<LinkTable> ref_link_table;
#endif
    } plan;

public:
	// Use both monolithic and incremental updates in order 
	// to build full db during analysis stage
    void InitialBuild();

	// Incremental strategy: perform updates on zones
    void Delete(const TreeZone &zone);
    void Insert(const TreeZone &zone);
    void InsertExtraZone(const TreeZone &extra_zone);
    void DeleteExtraZone(const TreeZone &extra_zone);

    void InitialWalk( const DBWalk::Actions *actions,
                      XLink root_xlink );

    XLink GetUniqueDomainExtension( Agent::TeleportResult tpr ) const;
    void ExtendDomainNewPattern( PatternLink root_plink );
    void ExtendDomainNewX();           
    void UnExtendDomain();
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

	const Indexes &GetIndexes() const;
	TreePtr<Node> GetRootNode() const;
	XLink GetRootXLink() const;

    void Dump() const;
#ifdef DB_ENABLE_COMPARATIVE_TEST
    void ExpectMatches() const;
#endif	
	
private:    
    // TreeKit implementation
  	set<LinkInfo> GetParents( TreePtr<Node> node ) const override;
  	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
  	
    void TestRelations();
    
  	XLink root_xlink;
    DBWalk db_walker;
};    
    
};

#endif
