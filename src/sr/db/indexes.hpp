#ifndef INDEXES_HPP
#define INDEXES_HPP

#include "../zone.hpp"
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
class LinkTable;
    
class Indexes
{
public:
    Indexes(shared_ptr<Lacing> lacing, const LinkTable *link_table, bool ref=false );
    
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
    void ExpectMatching( const Indexes &mut );
    void TestRelations( const unordered_set<XLink> &xlinks );

    // Category ordering TODO merge with SimpleCompare ordering
    typedef set<XLink, CategoryRelation> CategoryOrderedIndex;
    typedef CategoryOrderedIndex::iterator CategoryOrderedIt;

    // We will provide a SimpleCompare ordered version of the domain
    typedef set<XLink, SimpleCompareRelation> SimpleCompareOrderedIndex;
    typedef SimpleCompareOrderedIndex::iterator SimpleCompareOrderedIt;
    
    // We will provide a depth-first ordered version of the domain
    typedef set<XLink, DepthFirstRelation> DepthFirstOrderedIndex;
    typedef DepthFirstOrderedIndex::iterator DepthFirstOrderedIt;

    // Global domain of possible xlink values - new version
    DepthFirstOrderedIndex depth_first_ordered_index;            
    
    // Domain ordered by category
    CategoryOrderedIndex category_ordered_index;
    
    // Whole domain in here, grouped by simple compare, findable using eg lower_bound()
    // Should be the other way around, as an indication of policy
    SimpleCompareOrderedIndex simple_compare_ordered_index;   

private:
    const LinkTable *link_table;
    const bool ref;
    const bool use_incremental;
};    
    
}

#endif
