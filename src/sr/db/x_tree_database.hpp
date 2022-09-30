#ifndef X_TREE_DATABASE_HPP
#define X_TREE_DATABASE_HPP

#include "common/standard.hpp"
#include "../zone.hpp"
#include "../link.hpp"

#include "link_table.hpp"
#include "node_table.hpp"
#include "domain.hpp"
#include "indexes.hpp"

//#define DB_ENABLE_COMPARATIVE_TEST

namespace SR 
{    
    
class XTreeDatabase : public Traceable, 
                      public TreeKit
{
public:
    explicit XTreeDatabase( shared_ptr<Lacing> lacing );
    
private: 
    const struct Plan : public Traceable
    {
        Plan( shared_ptr<Lacing> lacing );
        
        shared_ptr<Domain> domain;
        shared_ptr<Indexes> indexes;
        shared_ptr<LinkTable> link_table;
        shared_ptr<NodeTable> node_table;

#ifdef DB_ENABLE_COMPARATIVE_TEST
        shared_ptr<Domain> ref_domain;
        shared_ptr<Indexes> ref_indexes;
        //shared_ptr<LinkTable> ref_link_table;
        //shared_ptr<NodeTable> ref_node_table;
#endif
    } plan;

public:
    void SetRoot(XLink root_xlink);

	// Use both monolithic and incremental updates in order 
	// to build full db during analysis stage
    void InitialBuild();

	// Monolithic strategy: clear for whole tree and build from scratch
    void MonolithicClear();
    void MonolithicBuild(); // includes permanent columns
	void MonolithicExtra(XLink extra_base_xlink);

	// Incremental strategy: perform updates on zones
    void InsertPermanent();
    void Delete(const TreeZone &zone);
    void Insert(const TreeZone &zone);
    void InsertExtra(XLink extra_base_xlink);
    void DeleteExtra(XLink extra_xlink);

    XLink UniquifyDomainExtension( TreePtr<Node> node, bool expect_in_domain );
    XLink FindDomainExtension( TreePtr<Node> node ) const;
    void ExtendDomainNewPattern( PatternLink root_plink );
    void ExtendDomainNewX();           
    void UnExtendDomain();

    const LinkTable::Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;    
    const NodeTable::Row &GetNodeRow(TreePtr<Node> node) const;
    bool HasNodeRow(TreePtr<Node> node) const;
	const Indexes &GetIndexes() const;
    void Dump() const;
#ifdef DB_ENABLE_COMPARATIVE_TEST
    void ExpectMatches() const;
#endif	

private:    
    // TreeKit implementation
  	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
  	
  	XLink root_xlink;
    DBWalk db_walker;
};    
    
};

#endif
