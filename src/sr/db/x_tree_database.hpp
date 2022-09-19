#ifndef XTREE_DATABASE_HPP
#define XTREE_DATABASE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "../sc_relation.hpp"
#include "helpers/simple_compare.hpp"
#include "helpers/transformation.hpp"
#include "tables.hpp"
#include "indexes.hpp"

#include <unordered_set>

class SimpleCompare;

namespace SYM
{
    class BooleanExpression;
};
    
/// SR namespace contains the search and replace implementation
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
        
        shared_ptr<Tables> tables;
    } plan;

public:
    void Clear();
    void FullBuild( XLink root_xlink );
    void UpdateRootXLink(XLink root_xlink);
    void BuildNonIncremental();
    void ClearIncremental(XLink base_xlink);
    void BuildIncremental(XLink base_xlink);
           
    void ExtendDomainNewPattern( PatternLink root_plink );
    void ExtendDomainNewX();           
            
    const Tables::Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;    
    const Tables::NodeRow &GetNodeRow(TreePtr<Node> node) const;
    bool HasNodeRow(TreePtr<Node> node) const;
	const Indexes &GetIndexes() const;
	Tables &GetTables();
	Indexes &GetIndexes();
	
private:    
    // TreeKit implementation
  	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
  	
  	XLink root_xlink;
};    
    
};

#endif
