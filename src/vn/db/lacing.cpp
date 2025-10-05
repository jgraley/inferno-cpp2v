#include "lacing.hpp"
#include "agents/agent.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"

#include "../tree/cpptree.hpp"

#include <limits>

#define BF

using namespace VN;    

Lacing::Lacing() :
    cats_to_lacing_range_lists()
{
}


void Lacing::Build( const set<const SYM::Expression *> &sub_exprs )
{    
    // Extract all the non-final archetypes from the IsInCategoryOperator nodes 
    // into a set so that they are uniqued by SimpleCompare equality. These
    // are the categories.
    Lacing::CategorySet raw_categories;
    for( auto sub_expr : sub_exprs )
    {
        if( auto in_cat_expr = dynamic_cast<const SYM::IsInCategoryOperator *>(sub_expr) )
        { 
            raw_categories.insert( in_cat_expr->GetArchetypeNode() );
        }
    }

    Build(raw_categories);
}


void Lacing::Build( const CategorySet &raw_categories_ )
{
    FixupCategories(raw_categories_);
    FindSuperAndSubCategories();
    Sort();
    BuildRanges();
    BuildDecisionTree();
    TestDecisionTree();
}


void Lacing::FixupCategories(const CategorySet &raw_categories_)
{
    // Uniquify using IsSameCategory() so we definitely get a
    // category-uniquification.
    categories.clear();
    for( TreePtr<Node> x : raw_categories_ )    
    {
        bool unique = true;
        for( TreePtr<Node> y : categories )
        {
            if( Node::IsEquivalentCategory(*x, *y) )
            {
                unique = false;
            }
            else
            {
                // A handy place to check that node ordering is always at least
                // as strict as catecory equivalence.
                ASSERT( Node::OrderCompare3Way( *x, *y, Orderable::TOTAL ) );
                ASSERT( Node::OrderCompare3Way( *x, *y, Orderable::REPEATABLE ) );
            }
		}
		
        if( unique )
        {
            categories.insert(x);
		}
    }
    
    

    // We need to process a NULL category that includes all the X tree
    // nodes that don't fall into any of the supplied categories. It's
    // a disjoint category: no strict supers or subs.
    categories.insert( nullptr ); 
    ncats = categories.size();
}


void Lacing::FindSuperAndSubCategories()
{
    // Find the set of strict/non-strict super/sub-categories for each category.
    for( TreePtr<Node> cat : categories )
    {
        cats_to_nonstrict_supercats[cat];
        cats_to_nonstrict_subcats[cat];
        cats_to_strict_supercats[cat];
        cats_to_strict_subcats[cat];
        for( TreePtr<Node> candidate : categories )
        {
            bool weak_super = LocalMatchWithNULL( candidate, cat );
            bool weak_sub = LocalMatchWithNULL( cat, candidate );
            if( weak_super )
                cats_to_nonstrict_supercats.at(cat).insert(candidate);
            if( weak_sub )
                cats_to_nonstrict_subcats.at(cat).insert(candidate);
            if( weak_super && !weak_sub )
                cats_to_strict_supercats.at(cat).insert(candidate);
            if( weak_sub && !weak_super )
                cats_to_strict_subcats.at(cat).insert(candidate);
        }
    }
}


void Lacing::Sort()
{
    // TODO optimisation opportunity: put all the metrics into 
    // a map<TPN, map<TPN, int>> first (n squared) and then just use that
    // in the main sort.
    
    // Put architypes into a vector.
    for( TreePtr<Node> arch : categories )
        cats_in_lacing_order.push_back( arch );
    
    // Determine lacing ordering, non-cyclic, weighted by number of 
    // strict subcategories. We will apply swaps of contiguous blocks of 
    // architypes for as long as we can reduce total distance metric on lace.
    int count_outer = 0;
    bool swapped;
    do
    {
        swapped = false;
        for( int bs=ncats/2; bs>=1; bs-- ) // NodeBlock sizes
        {
            for( int i=0; i+bs<=ncats; i++ )
            {
                ASSERT( i+bs <= ncats );
                for( int j=0; j+bs<=i; j++ )
                {
                    ASSERT( i-j >= bs );
                    ASSERT( j+bs <= ncats );
                    int curr_metric = 0, metric_if_swapped = 0;
                    
                    // If blocks touch, swaps will change neighbours and
                    // a different formula is required. 
                    if( i-j != ncats-bs ) 
                    {
                        curr_metric += GetMetric(i+bs-1, i+bs) + GetMetric(j, j-1);
                        metric_if_swapped += GetMetric(i, j-1) + GetMetric(j+bs-1, i+bs);
                    }
                    else
                    {
                        curr_metric += 0;
                        metric_if_swapped += 0;
                    }
                    
                    if( i-j != bs ) 
                    {
                        curr_metric += GetMetric(i, i-1) + GetMetric(j+bs-1, j+bs);
                        metric_if_swapped += GetMetric(i+bs-1, j+bs) + GetMetric(j, i-1);
                    }
                    else
                    {
                        curr_metric += GetMetric(j+bs-1, i);
                        metric_if_swapped += GetMetric(i+bs-1, j);
                    }
                    
                    if( metric_if_swapped < curr_metric )
                    {
                        int expected_reduction = curr_metric - metric_if_swapped;
                        int tot=0;
                        vector<int> old_metrics;
                        for( int k=0; k<ncats; k++ )               
                        {         
                            old_metrics.push_back(GetMetric(k, k+1));                        
                            tot += GetMetric(k, k+1);
                        }
                        
                        for( int k=0; k<bs; k++ )
                            cats_in_lacing_order.at(i+k).swap( cats_in_lacing_order.at(j+k) );
                        
                        for( int k=0; k<ncats; k++ )                        
                            tot -= GetMetric(k, k+1);                        
                        TRACEC("Swap i=%d j=%d bs=%d expecting %d (%d-%d) got %d\n", i, j, bs, expected_reduction, curr_metric, metric_if_swapped, tot);
                        if( expected_reduction != tot )
                        {
                            for( int k=0; k<ncats; k++ )
                                TRACEC("%d: %d -> %d\n", k, old_metrics[k], GetMetric(k, k+1) );
                        }
                        ASSERT( expected_reduction==tot );
                        swapped = true;
                    }
                }
            }
        }
        count_outer++;        
    } while( swapped );    
    TRACE("count_outer=%d\n", count_outer);
    
    // Shows ordering and supercats together 
    if( Tracer::IsEnabled() )
    {
        vector< tuple<int, TreePtr<Node>, CategorySet>> for_trace;
        for( int i=0; i<ncats; i++ )
        {
            auto arch = cats_in_lacing_order.at(i);
            for_trace.push_back( make_tuple( GetMetric(i, i+1), arch, cats_to_strict_supercats.at(arch) ) );
        }
        TRACE(for_trace)("\n");
    }    
}    


int Lacing::GetMetric(int p, int q)
{
    // Non-cyclic, so if we go off the end the score is zero
    if( p<0 || p>=ncats || q<0 || q>=ncats )
        return 0;
    ASSERT( p != q ); // p is candidate to swap and q is neighbour. Swapping p must not change q.
    const TreePtr<Node> &pa = cats_in_lacing_order.at(p);
    const TreePtr<Node> &qa = cats_in_lacing_order.at(q);
    const CategorySet &psupers = cats_to_nonstrict_supercats.at(pa);
    const CategorySet &qsupers = cats_to_nonstrict_supercats.at(qa);
    CategorySet diff = SymmetricDifferenceOf( psupers, qsupers );
    int metric = 0;
    for( TreePtr<Node> t : diff )
        metric += cats_to_nonstrict_subcats.at(t).size(); // weight for bigness of the categories in the diff
    //TRACEC("p=%d q=%d: ", p, q)(pa)(" ")(qa)(" metric=%d\n", metric);
    return metric;
}


void Lacing::BuildRanges()
{
    // Determine a list of ranges for each category within the lacing:
    // pairs of (begin, end) indices on cats_in_lacing_order, i.e. half-open. 
    CategorySet prev_supercats; 
    for( int i=0; i<=ncats; i++ ) // Note extra iteration! Lets us use ncats as overall "end".
    {
        TreePtr<Node> cat_i = (i<ncats ? cats_in_lacing_order.at(i) : nullptr);
        CategorySet current_supercats = (cat_i ? cats_to_nonstrict_supercats.at(cat_i) : CategorySet());                
            
        CategorySet begin_supercats = DifferenceOf( current_supercats, prev_supercats ); // inclusive
        for( TreePtr<Node> supercat : begin_supercats )
            cats_to_lacing_range_lists[supercat].push_back( make_pair(i, -1) );

        CategorySet end_supercats = DifferenceOf( prev_supercats, current_supercats ); // exclusive
        for( TreePtr<Node> supercat : end_supercats )
            cats_to_lacing_range_lists[supercat].back().second = i;  
                 
        prev_supercats = current_supercats;
    }    
    TRACE(cats_to_lacing_range_lists)("\n");      
}


void Lacing::BuildDecisionTree()
{
    // Gather sets of lacing ordinals.
    for( int i=0; i<ncats; i++ )
    {
        TreePtr<Node> cat_i = cats_in_lacing_order.at(i);
        CategorySet current_supercats = cats_to_nonstrict_supercats.at(cat_i);                
        for( TreePtr<Node> supercat : current_supercats )
            cats_to_lacing_sets[supercat].insert(i);
    }    
    TRACE(cats_to_lacing_sets)("\n");
    
    // Generate a decision tree that determines lacing ordinal using just
    // IsSubcategory() on some X node (not necessarily seen here)
    set<int> possible_lacing_ordinals;
    for( int i=0; i<ncats; i++ )
        possible_lacing_ordinals.insert(i);
        
    decision_tree_root = MakeDecisionSubtree( possible_lacing_ordinals );
    
    TRACE("Decision tree to obtain lacing ordinal given any X node\n")
         ("\n"+decision_tree_root->Render());
}


void Lacing::TestDecisionTree()
{
    // Self-test
    for( int i=0; i<ncats; i++ ) 
    {
        TreePtr<Node> cat = cats_in_lacing_order.at(i);
        ASSERT( GetOrdinalForNode( cat ) == i );
    }
    TRACE("Lacing decision tree self-check OK\n");
}


shared_ptr<Lacing::DecisionNode> Lacing::MakeDecisionSubtree( const set<int> &possible_lacing_ordinals )
{    
    ASSERT( !possible_lacing_ordinals.empty() ); // Unexpected error: assert on best_cat should prevent this from failing
    
    // Deal with termination scenarios first: only one possible lacing ordinal
    if( possible_lacing_ordinals.size()==1 )
    {
        // Get that ordinal
        int ordinal = SoloElementOf( possible_lacing_ordinals );
        
        // Generate leaf node during unwind
        return make_shared<DecisionNodeLeaf>( this, ordinal );
    }
    
    // Find the category whose lacing set best halves the set of possible lacing indices
    int target_inter_size = possible_lacing_ordinals.size(); // TIMES TWO!!
    int best_diff = target_inter_size+1; // TIMES TWO!!
    TreePtr<Node> best_cat;  
    set<int> best_inter;  
    bool found = false;
    //TRACEC(possible_lacing_ordinals)(" target=%d\n", target_inter_size); 
    for( TreePtr<Node> cat : categories )
    {
        // We don't want to test the NULL category, so hopefully the others
        // are sufficient to differentiate everything.
        if( !cat )
            continue; 
            
        // size of intersection between current lacing set and possible indices
        set<int> inter = IntersectionOf( possible_lacing_ordinals,
                                         cats_to_lacing_sets.at(cat) );
        //TRACEC(cat)(" intersection=")(inter)("\n");
        
        // Skip where cat didn't provide any useful split of possible indices
        if( inter.empty() || inter.size()==possible_lacing_ordinals.size() )
            continue;
                                         
        // Closer to the target is better
        int diff = abs( (int)inter.size()*2 - target_inter_size ); // TIMES TWO!!
        if( diff < best_diff )
        {
            best_diff = diff;
            best_cat = cat;
            best_inter = inter; 
            found = true;
        }                                                                               
    }
        
    ASSERT( found )
          ("Problem with the lacing: no category can differentiate these possible indices:\n")
          (possible_lacing_ordinals)("\n");
        
    // We'll also need a set of possible indices that are NOT in best lacing set
    set<int> best_setdiff = DifferenceOf( possible_lacing_ordinals,
                                          cats_to_lacing_sets.at(best_cat) );

    // Recurse twice on the two hopefully-nearly-halves of the possible set
    shared_ptr<DecisionNode> yes = MakeDecisionSubtree( best_inter );
    shared_ptr<DecisionNode> no = MakeDecisionSubtree( best_setdiff );
    
    // Use the set<int>'s ordering to get min and max elements easily
    int min_lacing_ordinal = FrontOf(possible_lacing_ordinals);
    int max_lacing_ordinal = BackOf(possible_lacing_ordinals);
 
    // Generate a decision node that should decide based on IsSubcategory
    // during the unwind.
   return make_shared<DecisionNodeLocalMatch>( this, best_cat, yes, no, min_lacing_ordinal, max_lacing_ordinal );    
}


bool Lacing::LocalMatchWithNULL( TreePtr<Node> l, TreePtr<Node> r )
{
    // Model TreePtr<Node>==NULL as a disjoint category
    if( !l || !r )
        return false; // Null and non-NULL never match 

    if( !r && !l )
		return true; // NULL matches NULL
               	
	auto p = make_pair(type_index(typeid(*l)), type_index(typeid(*r)));
	SubCategoryCache::iterator it = subcategory_cache.find( p );
	if( it != subcategory_cache.end() ) // Found
	{
		return it->second;
	}
	else // Not found
	{
        bool subcat = l->IsSubcategory( *r );
        InsertSolo( subcategory_cache, make_pair(p, subcat) );
        return subcat;
	}
}


Lacing::DecisionNode::DecisionNode(Lacing *lacing_) :
	lacing( lacing_ )
{
}


Lacing::DecisionNode::~DecisionNode()
{
}


Lacing::DecisionNodeLocalMatch::DecisionNodeLocalMatch( Lacing *lacing_,
                                                        TreePtr<Node> category_, 
                                                        shared_ptr<DecisionNode> if_yes_,         
                                                        shared_ptr<DecisionNode> if_no_,
                                                        int min_lacing_ordinal_,
                                                        int max_lacing_ordinal_ ) :
    DecisionNode( lacing_ ),                                                   
    category( category_ ),
    if_yes( if_yes_ ),
    if_no( if_no_ ),
    min_lacing_ordinal( min_lacing_ordinal_ ),
    max_lacing_ordinal( max_lacing_ordinal_ )
{
}
    

const Lacing::DecisionNode *Lacing::DecisionNodeLocalMatch::GetNextDecisionNode( TreePtr<Node> node ) const
{
    return lacing->LocalMatchWithNULL( category, node ) ? if_yes.get() : if_no.get();
}


pair<int, int> Lacing::DecisionNodeLocalMatch::GetLacingRange() const 
{
    return make_pair( min_lacing_ordinal, max_lacing_ordinal );
}


string Lacing::DecisionNodeLocalMatch::Render(string pre)
{
    string s = "Are you a kind of " + Trace(category) + "?\n";
    s += pre + "✔ ⇒ " + if_yes->Render(pre+"    ");
    s += pre + "✘ ⇒ " + if_no ->Render(pre+"    ");
    return s;
}
        

Lacing::DecisionNodeLeaf::DecisionNodeLeaf( Lacing *lacing_,
                                            int lacing_ordinal_ ) :
    DecisionNode( lacing_ ),                                                   
    lacing_ordinal( lacing_ordinal_ )
{
}


int Lacing::DecisionNodeLeaf::GetLacingOrdinal() const
{
    return lacing_ordinal;
}


string Lacing::DecisionNodeLeaf::Render(string pre)
{
    return pre + SSPrintf(" your lacing ordinal is %d\n", lacing_ordinal );
}
        
        
const list<pair<int, int>> &Lacing::TryGetRangeListForCategory( TreePtr<Node> archetype ) const
{
    if( cats_to_lacing_range_lists.count(archetype)>0 )
    {
        auto &lrl = cats_to_lacing_range_lists.at(archetype);
        ASSERT( !lrl.empty() );
        return lrl;
    }
    else
    {
        // Empty indicates failure
        static const list<pair<int, int>> empty_lrl;
        return empty_lrl;
    }
}


const list<pair<int, int>> &Lacing::GetRangeListForCategory( TreePtr<Node> archetype ) const
{
    auto &lrl = TryGetRangeListForCategory( archetype );
    ASSERT( !lrl.empty() )
          ("Could not find lacing info for ")(archetype)
          ("\nin:\n")(cats_to_lacing_range_lists)("\n")
          ("Did you leave NODE_FUNCTIONS_FINAL or NODE_FUNCTIONS out of a node decl?\n")
          ("That sould make the node equivalent to its parent node wrt cat ordering\n")
          ("and so one of them will get dropped from the lacing process.");
    return lrl;
}


int Lacing::GetOrdinalForNode( TreePtr<Node> target_node ) const
{
    const Lacing::DecisionNode *decision_node = decision_tree_root.get();
    while(true) 
    {
        if( auto dn_local_match = dynamic_cast<const DecisionNodeLocalMatch *>(decision_node) )
        {
            //dn_local_match->GetLacingRange();
            decision_node = dn_local_match->GetNextDecisionNode( target_node );
        }
        else if( auto dn_leaf = dynamic_cast<const DecisionNodeLeaf *>(decision_node) )
        {
            return dn_leaf->GetLacingOrdinal();
        }
        else
        {
            ASSERTFAIL();            
        }
    }
}


Orderable::Diff Lacing::OrdinalCompare( TreePtr<Node> lnode, TreePtr<Node> rnode ) const 
{
    const Lacing::DecisionNode *decision_node_l = decision_tree_root.get();
    const Lacing::DecisionNode *decision_node_r = decision_tree_root.get();
    int lmin=0, lmax=numeric_limits<int>::max(), 
        rmin=0, rmax=numeric_limits<int>::max();
    while(true) 
    {
        // Advance left
        if( !decision_node_l )
        {
			// min and max are equal, nothing more to do on the left
		}
        else if( auto dn_local_match_l = dynamic_cast<const DecisionNodeLocalMatch *>(decision_node_l) )
        {
            decision_node_l = dn_local_match_l->GetNextDecisionNode( lnode );
            tie(lmin, lmax) = dn_local_match_l->GetLacingRange();
        }
        else if( auto dn_leaf_l = dynamic_cast<const DecisionNodeLeaf *>(decision_node_l) )
        {
            lmin = lmax = dn_leaf_l->GetLacingOrdinal();               
        }
        else 
        {
            ASSERTFAIL();            
        }
        
        if( lmin == lmax )
			decision_node_l = nullptr;   
       
        // Do we have a result yet?
        if( lmax < rmin )
            return lmax-rmin;
        if( lmin > rmax )
            return lmin-rmax;
        if( lmin == lmax && rmin == rmax && lmin==rmin )
            return 0;

        // Advance right
        if( !decision_node_r )
        {
			// min and max are equal, nothing more to do on the right
		}
        else if( auto dn_local_match_r = dynamic_cast<const DecisionNodeLocalMatch *>(decision_node_r) )
        {
            decision_node_r = dn_local_match_r->GetNextDecisionNode( rnode );
            tie(rmin, rmax) = dn_local_match_r->GetLacingRange();
        }
        else if( auto dn_leaf_r = dynamic_cast<const DecisionNodeLeaf *>(decision_node_r) )
        {
            rmin = rmax = dn_leaf_r->GetLacingOrdinal();
        }
        else 
        {
            ASSERTFAIL();            
        }       

        if( rmin == rmax )
			decision_node_r = nullptr;   
			
		// Do we have a result yet?
        if( lmax < rmin )
            return lmax-rmin;
        if( lmin > rmax )
            return lmin-rmax;
        if( lmin == lmax && rmin == rmax && lmin==rmin )
            return 0;
    }
}
