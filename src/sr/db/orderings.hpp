#ifndef ORDERINGS_HPP
#define ORDERINGS_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "sc_relation.hpp"
#include "cat_relation.hpp"
#include "df_relation.hpp"
#include "db_walk.hpp"
#include "node_table.hpp"

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

    void MainTreeInsert(TreeZone &zone, const DBCommon::CoreInfo *base_info, bool do_intrinsics);
    void MainTreeDelete(TreeZone &zone, const DBCommon::CoreInfo *base_info, bool do_intrinsics);

private:
	void InsertAction(const DBWalk::WalkInfo &walk_info, bool do_intrinsics);
    void DeleteAction(const DBWalk::WalkInfo &walk_info, bool do_intrinsics);

	set<TreePtr<Node>> GetTerminusAndBaseAncestors( const TreeZone &tz ) const; 

public:        
    void Dump() const;
    void CheckRelations( const vector<XLink> &xlink_domain,  
                         const vector<TreePtr<Node>> &node_domain );
    static void CheckEqual( shared_ptr<Orderings> l, shared_ptr<Orderings> r, bool intrinsic );

    // Category ordering TODO merge with SimpleCompare ordering
    typedef set<CategoryRelation::KeyType, 
                CategoryRelation> CategoryOrdering;

    // We will provide a SimpleCompare ordered version of the domain
    typedef set<SimpleCompareRelation::KeyType, 
                SimpleCompareRelation> SimpleCompareOrdering;
    
    // We will provide a depth-first ordered version of the domain
    // Why not use the X tree directly? Well, we have the DepthFirstRelation
    // but would need to code up an extensionaliser that uses the X tree 
    // (not too hard). Real reason is to keep the door open to future 
    // combined orderings.
    typedef set<DepthFirstRelation::KeyType, 
                DepthFirstRelation> DepthFirstOrdering;

    // Global domain of possible xlink values - new version
    DepthFirstOrdering depth_first_ordering;            
    
    // Domain ordered by category
    CategoryOrdering category_ordering;
    
    // Whole domain in here, grouped by simple compare, findable using eg lower_bound()
    // Should be the other way around, as an indication of policy
    SimpleCompareOrdering simple_compare_ordering;   

private:
    const XTreeDatabase *db;
    DBWalk db_walker;
    
    map<TreePtr<Node>, size_t> node_ref_counts;
    
    // Note: live across deleting walks
    map<TreePtr<Node>, size_t> node_reached_count;    
    set<TreePtr<Node>> debt;
};    
    
}

#endif
