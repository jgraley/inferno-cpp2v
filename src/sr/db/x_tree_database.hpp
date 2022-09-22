#ifndef X_TREE_DATABASE_HPP
#define X_TREE_DATABASE_HPP

#include "common/standard.hpp"
#include "../zone.hpp"
#include "../link.hpp"

#include "link_table.hpp"
#include "node_table.hpp"
#include "domain.hpp"
#include "indexes.hpp"

#define DB_TEST_EXTENSIONS

namespace SR 
{
class SimpleCompareQuotientSet;
class VNStep;
class Lacing;
    
class XTreeDatabase : public Traceable, 
                      public TreeKit
{
public:
    explicit XTreeDatabase( const set< shared_ptr<SYM::BooleanExpression> > &clauses = {} );
    
private: 
    const struct Plan : public Traceable
    {
        Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses );
        
        shared_ptr<Domain> domain;
        shared_ptr<Indexes> indexes;
        shared_ptr<LinkTable> link_table;
        shared_ptr<NodeTable> node_table;

#ifdef DB_TEST_EXTENSIONS
        shared_ptr<Domain> ref_domain;
        shared_ptr<Indexes> ref_indexes;
        //shared_ptr<LinkTable> ref_link_table;
        //shared_ptr<NodeTable> ref_node_table;
#endif
    } plan;

public:
    void SetRoot(XLink root_xlink);

    void FullClear();
    void FullBuild();

    void ClearMonolithic();
    void BuildMonolithic();

    void BuildInit();
    void Delete(const TreeZone &zone);
    void Insert(const TreeZone &zone);
           
    XLink UniquifyDomainExtension( XLink xlink );
    XLink FindDomainExtension( XLink xlink ) const;
    void ExtendDomainNewPattern( PatternLink root_plink );
    void ExtendDomainNewX();           
            
    const LinkTable::Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;    
    const NodeTable::Row &GetNodeRow(TreePtr<Node> node) const;
    bool HasNodeRow(TreePtr<Node> node) const;
	const Indexes &GetIndexes() const;
#ifdef DB_TEST_EXTENSIONS
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
