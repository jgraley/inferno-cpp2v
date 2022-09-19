#ifndef X_TREE_DATABASE_HPP
#define X_TREE_DATABASE_HPP

#include "common/standard.hpp"
#include "../link.hpp"

#include "tables.hpp"
#include "domain.hpp"
#include "indexes.hpp"

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
        
        shared_ptr<Indexes> indexes;
        shared_ptr<Domain> domain;
        shared_ptr<Tables> tables;
    } plan;

public:
    void Clear();
    void FullBuild( XLink root_xlink );
    void UpdateRootXLink(XLink root_xlink);
    void BuildNonIncremental();
    void ClearIncremental(XLink base_xlink);
    void BuildIncremental(XLink base_xlink);
           
    XLink UniquifyDomainExtension( XLink xlink );
    XLink FindDomainExtension( XLink xlink ) const;
    void ExtendDomainNewPattern( PatternLink root_plink );
    void ExtendDomainNewX();           
            
    const Tables::Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;    
    const Tables::NodeRow &GetNodeRow(TreePtr<Node> node) const;
    bool HasNodeRow(TreePtr<Node> node) const;
	const Indexes &GetIndexes() const;
	Indexes &GetIndexes();
	
private:    
    // TreeKit implementation
  	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
  	
  	XLink root_xlink;
};    
    
};

#endif
