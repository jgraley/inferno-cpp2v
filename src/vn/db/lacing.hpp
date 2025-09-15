#ifndef LACING_HPP
#define LACING_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "helpers/simple_compare.hpp"

#include <unordered_set>

class SimpleCompare;
   
namespace SYM
{
    class Expression;
};
    
namespace SR 
{   
/*
 * Lacing provides the tools and data structures required to create an ordering 
 * on categories of node types. Categories and their relationships are boiled
 * down to sets of integers which are (as far as possible) one or a small
 * number of contiguous ranges. We do not require access to all defined node 
 * types or even all categories - just the ones we're likely to restrict against
 * during search. 
 * 
 * Methods are provided for building/updating the ordering (GetOrdinalForNode())
 * and for generating ranges from categories for AllX operators to use with
 * eg upper_bound() etc (GetRangeListForCategory()).
 */
class Lacing
{
public:    
    typedef set<TreePtr<Node>> CategorySet;   
    typedef unordered_map<pair<type_index, type_index>, bool, PairHash> SubCategoryCache;

    Lacing();

    // Build lacing strucutres based on clausal expressions from the patterns
    void Build( const set<const SYM::Expression *> &sub_exprs );

    // Build the lacing data structures including decision tree. Only needed
    // once, at planning time for the whole sequence.
    void Build( const CategorySet &raw_categories );

private:    
    class DecisionNode;
    
    void FixupCategories(const CategorySet &categories_);
    void FindSuperAndSubCategories();
    void Sort();
    int GetMetric(int i, int j);
    void BuildRanges();
    void BuildDecisionTree();
    void TestDecisionTree();
    shared_ptr<DecisionNode> MakeDecisionSubtree( const set<int> &possible_lacing_ordinals );
    bool LocalMatchWithNULL( TreePtr<Node> l, TreePtr<Node> r );

    class DecisionNode
    {        
    public:
		DecisionNode(Lacing *lacing_);
        virtual ~DecisionNode();
        virtual string Render(string pre="") = 0;
    protected:
        Lacing * const lacing;
    };
    
    class DecisionNodeLocalMatch : public DecisionNode
    {
    public:
        DecisionNodeLocalMatch( Lacing *lacing_,
                                TreePtr<Node> category, 
                                shared_ptr<DecisionNode> if_yes,         
                                shared_ptr<DecisionNode> if_no,
                                int min_lacing_ordinal,
                                int max_lacing_ordinal );
        const DecisionNode *GetNextDecisionNode( TreePtr<Node> node ) const;
        pair<int, int> GetLacingRange() const;
        string Render(string pre) override;
        
    private:
        TreePtr<Node> category;
        shared_ptr<DecisionNode> if_yes;
        shared_ptr<DecisionNode> if_no;        
        int min_lacing_ordinal;
        int max_lacing_ordinal;
    };
    
    class DecisionNodeLeaf : public DecisionNode
    {
    public:
        DecisionNodeLeaf( Lacing *lacing_,
                          int lacing_ordinal );
        int GetLacingOrdinal() const;
        string Render(string pre) override;
        
    private:
        int lacing_ordinal;
    };    
    
public:
    // (Tries to) return a list of non-overlapping pair(begin, end) ranges in increasing order.
    // These are the possible values of lacing ordinal that will match the category
    // defined by the supplied archetype.
    const list<pair<int, int>> &TryGetRangeListForCategory( TreePtr<Node> archetype ) const;
    const list<pair<int, int>> &GetRangeListForCategory( TreePtr<Node> archetype ) const;
    
    // Returns the lacing ordinal value for the candidate.
    int GetOrdinalForNode( TreePtr<Node> node ) const;

    // Approximately GetOrdinalForNode(lnode) - GetOrdinalForNode(rnode)
    // but close enough for comparisons
    Orderable::Diff OrdinalCompare( TreePtr<Node> lnode, TreePtr<Node> rnode ) const;

private:    
    CategorySet categories;
    int ncats;
    // Categorisation of the categories
    map<TreePtr<Node>, CategorySet> cats_to_nonstrict_supercats;
    map<TreePtr<Node>, CategorySet> cats_to_nonstrict_subcats;
    map<TreePtr<Node>, CategorySet> cats_to_strict_supercats;
    map<TreePtr<Node>, CategorySet> cats_to_strict_subcats;
    // Lacing ordering
    vector<TreePtr<Node>> cats_in_lacing_order;
    // Lacing range sets
    map<TreePtr<Node>, list<pair<int, int>>, SimpleCompare> cats_to_lacing_range_lists;
    // Decision tree stuff
    map<TreePtr<Node>, set<int>> cats_to_lacing_sets;
    shared_ptr<DecisionNode> decision_tree_root;
    SubCategoryCache subcategory_cache;
};
    
};

#endif
