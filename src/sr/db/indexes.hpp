#ifndef INDEXES_HPP
#define INDEXES_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "../sc_relation.hpp"
#include "db_walk.hpp"

namespace SYM
{
    class BooleanExpression;
};
    

namespace SR 
{
class SimpleCompareQuotientSet;
class VNStep;
class Lacing;
class XTreeDatabase;
    
class Indexes
{
public:
    explicit Indexes( const set< shared_ptr<SYM::BooleanExpression> > &clauses = {} );
    
    typedef int OrdinalType;

private: 
    const struct Plan : public Traceable
    {
        Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses );
        
        shared_ptr<Lacing> lacing;

    } plan;
    
public:
    class CategoryRelation
    {
    public:
        CategoryRelation();
        CategoryRelation( shared_ptr<Lacing> lacing );
        CategoryRelation& operator=(const CategoryRelation &other);
        
        bool operator() (const XLink& x_link, const XLink& y_link) const;
    private:
        shared_ptr<Lacing> lacing;
    };

    // Create a node here so that a regular XLink can be used and passed
    // through the sym stuff by value.
    class CategoryMinimaxNode : public Node
    {
    public:
        NODE_FUNCTIONS_FINAL
        CategoryMinimaxNode( int lacing_ordinal );
        CategoryMinimaxNode(); ///< default constructor, for making archetypes 
        int GetLacingOrdinal() const;
        string GetTrace() const override;
    private:
        int lacing_ordinal;
    };

    const Lacing *GetLacing() const;

	void PopulateActions(DBWalk::Actions &actions);
	void PrepareFullBuild(DBWalk::Actions &actions);
	void PrepareExtraXLink(DBWalk::Actions &actions);

    // Domain ordered by depth-first walk
    // Don't use a vector for this:
    // (a) you'd need the size in advance otherwise the iterators in
    // the xlink_table will go bad while populating and
    // (b) incremental domain update will be hard
    typedef list<XLink> DepthFirstOrderedIndex;    
    typedef DepthFirstOrderedIndex::const_iterator DepthFirstOrderedIt;    
    
    // Category ordering TODO merge with SimpleCompare ordering
    typedef multiset<XLink, CategoryRelation> CategoryOrderedIndex;
    typedef CategoryOrderedIndex::iterator CategoryOrderedIt;

    // We will provide a SimpleCompare ordered version of the domain
    typedef multiset<XLink, SimpleCompareRelation> SimpleCompareOrderedIndex;
    typedef SimpleCompareOrderedIndex::iterator SimpleCompareOrderedIt;
    
    // Global domain of possible xlink values - ordered
    DepthFirstOrderedIndex depth_first_ordered_index;            
    
    // Domain ordered by category
    CategoryOrderedIndex category_ordered_index;
    
    // Whole domain in here, grouped by simple compare, findable using eg lower_bound()
    // Should be the other way around, as an indication of policy
    SimpleCompareOrderedIndex simple_compare_ordered_index;   
};    
    
}

#endif
