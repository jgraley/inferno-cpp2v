#ifndef ORDERINGS_HPP
#define ORDERINGS_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "sc_relation.hpp"
#include "cat_relation.hpp"
#include "df_relation.hpp"
#include "db_walk.hpp"

namespace SYM
{
    class BooleanExpression;
};
    
    
namespace SR 
{
class Lacing;
class XTreeDatabase;
    
class Orderings
{
public:
    Orderings(shared_ptr<Lacing> lacing, const XTreeDatabase *db );
    
private: 
    const struct Plan : public Traceable
    {
        Plan( shared_ptr<Lacing> lacing );
        
        shared_ptr<Lacing> lacing;

    } plan;
    
public:
    const Lacing *GetLacing() const;

    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);
    
    void Dump() const;
    void TestRelations( const unordered_set<XLink> &xlinks );

    // Category ordering TODO merge with SimpleCompare ordering
    typedef set<XLink, CategoryRelation> CategoryOrdering;
    typedef CategoryOrdering::iterator CategoryOrderingIterator;

    // We will provide a SimpleCompare ordered version of the domain
    typedef set<XLink, SimpleCompareRelation> SimpleCompareOrdering;
    typedef SimpleCompareOrdering::iterator SimpleCompareOrderingIterator;
    
    // We will provide a depth-first ordered version of the domain
    typedef set<XLink, DepthFirstRelation> DepthFirstOrdering;
    typedef DepthFirstOrdering::iterator DepthFirstOrderingIterator;

    // Global domain of possible xlink values - new version
    DepthFirstOrdering depth_first_ordering;            
    
    // Domain ordered by category
    CategoryOrdering category_ordering;
    
    // Whole domain in here, grouped by simple compare, findable using eg lower_bound()
    // Should be the other way around, as an indication of policy
    SimpleCompareOrdering simple_compare_ordering;   

private:
    const XTreeDatabase *db;
};    
    
}

#endif