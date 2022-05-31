#include "lacing.hpp"
#include "sc_relation.hpp"
#include "agents/agent.hpp"

using namespace SR;    

Lacing::Lacing() 
{
}


void Lacing::Build( const CategorySet &categories_ )
{
    categories = categories_;
    
    // Find the set of strict/non-strict super/sub-categories for each category.
    for( TreePtr<Node> cat : categories )
    {
        cats_to_nonstrict_supercats.emplace( std::piecewise_construct,
                                        std::forward_as_tuple(cat),
                                        std::forward_as_tuple(SimpleCompare()) );
        cats_to_nonstrict_subcats.emplace( std::piecewise_construct,
                                      std::forward_as_tuple(cat),
                                      std::forward_as_tuple(SimpleCompare()) );
        cats_to_strict_supercats.emplace( std::piecewise_construct,
                                        std::forward_as_tuple(cat),
                                        std::forward_as_tuple(SimpleCompare()) );
        cats_to_strict_subcats.emplace( std::piecewise_construct,
                                      std::forward_as_tuple(cat),
                                      std::forward_as_tuple(SimpleCompare()) );
        for( TreePtr<Node> candidate : categories )
        {
            bool weak_super = candidate->IsLocalMatch( cat.get() );
            bool weak_sub = cat->IsLocalMatch( candidate.get() );
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

    // Put architypes into a vector.
    for( TreePtr<Node> arch : categories )
    {
        cats_in_lacing_order.push_back( arch );
    }
    
    // Determine lacing ordering, non-cyclic, weighted by number of 
    // strict subcategories. We will apply swaps of contiguous blocks of 
    // architypes for as long as we can reduce total distance metric on lace.
    int n = cats_in_lacing_order.size();
    int count_outer = 0;
    bool swapped;
    do
    {
        swapped = false;
        for( int bs=n/2; bs>=1; bs-- ) // Block sizes
        {
            for( int i=0; i+bs<=n; i++ )
            {
                for( int j=0; j+bs<=i; j++ )
                {
                    ASSERT( i-j >= bs );
                    int curr_metric = 0, metric_if_swapped = 0;
                    
                    // If blocks touch, swaps will change neigbours and
                    // a different formula is required. 
                    if( i-j != n-bs ) 
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
                        for( int k=0; k<n; k++ )               
                        {         
                            old_metrics.push_back(GetMetric(k, k+1));                        
                            tot += GetMetric(k, k+1);
                        }
                        
                        for( int k=0; k<bs; k++ )
                            cats_in_lacing_order.at((i+k)%n).swap( cats_in_lacing_order.at((j+k)%n) );
                        
                        for( int k=0; k<n; k++ )                        
                            tot -= GetMetric(k, k+1);                        
                        TRACEC("Swap i=%d j=%d bs=%d expecting %d (%d-%d) got %d\n", i, j, bs, expected_reduction, curr_metric, metric_if_swapped, tot);
                        if( expected_reduction != tot )
                        {
                            for( int k=0; k<n; k++ )
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
        for( int i=0; i<n; i++ )
        {
            auto arch = cats_in_lacing_order.at(i);
            for_trace.push_back( make_tuple( GetMetric(i, i+1), arch, cats_to_strict_supercats.at(arch) ) );
        }
        TRACE(for_trace)("\n");
    }
    
    // Determine a list of ranges for each category within the lacing:
    // pairs of (begin, end) indices on cats_in_lacing_order. Also gather
    // sets of lacing indexes.
    CategorySet prev_cats; 
    for( int i=0; i<=n; i++ ) // note extra iteration!!
    {
        TreePtr<Node> cat_i = (i<n ? cats_in_lacing_order.at(i) : nullptr);
        CategorySet current_cats = (cat_i ? cats_to_nonstrict_supercats.at(cat_i) : CategorySet());                
        for( TreePtr<Node> cat : current_cats )
            cats_to_lacing_sets[cat].insert(i);
            
        CategorySet begin_cats = DifferenceOf( current_cats, prev_cats );
        for( TreePtr<Node> cat : begin_cats )
            cats_to_lacing_range_lists[cat].push_back( make_pair(i, -1) );

        CategorySet end_cats = DifferenceOf( prev_cats, current_cats );
        for( TreePtr<Node> cat : end_cats )
            cats_to_lacing_range_lists[cat].back().second = i;  
                 
        prev_cats = current_cats;
    }    
    TRACE(cats_to_lacing_sets)("\n");
    TRACE(cats_to_lacing_range_lists)("\n");  
    
    // Generate a decision tree that determines lacing index using just
    // IsLocalMatch() on some X node (not necessarily seen here)
    set<int> possible_lacing_indices;
    for( int i=0; i<n; i++ )
        possible_lacing_indices.insert(i);
    decision_tree = MakeDecisionTree( possible_lacing_indices );
    TRACE("Decision tree to obtain lacing index given any X node\n")
         ("\n"+decision_tree->Render());
         
    // Self-test
    for( int i=0; i<n; i++ ) 
    {
        TreePtr<Node> cat = cats_in_lacing_order.at(i);
        ASSERT( GetIndexForCandidate( cat ) == i );
    }
    TRACE("Lacing self-check OK\n");
}


const list<pair<int, int>> &Lacing::GetRangeListForCategory( TreePtr<Node> archetype ) const
{
    return cats_to_lacing_range_lists.at(archetype);
}


int Lacing::GetIndexForCandidate( TreePtr<Node> candidate ) const
{
    return decision_tree->GetLacingIndex( candidate );
}


int Lacing::GetMetric(int p, int q)
{
    int n = cats_in_lacing_order.size();
    // Non-cyclic, so if we go off the end the score is zero
    if( p<0 || p>=n || q<0 || q>=n )
        return 0;
    ASSERT( p != q ); // p is candidate to swap and q is neighbour. Swapping p must not change q.
    const TreePtr<Node> &pa = cats_in_lacing_order.at(p);
    const TreePtr<Node> &qa = cats_in_lacing_order.at(q);
    const CategorySet &psupers = cats_to_nonstrict_supercats.at(pa);
    const CategorySet &qsupers = cats_to_nonstrict_supercats.at(qa);
    CategorySet diff = SymmetricDifferenceOf( psupers, qsupers );
    int metric = 0;
    for( TreePtr<Node> t : diff )
        metric += cats_to_strict_subcats.at(t).size(); // weight for bigness of the categories in the diff
    //TRACEC("p=%d q=%d: ", p, q)(pa)(" ")(qa)(" metric=%d\n", metric);
    return metric;
}


shared_ptr<Lacing::DecisionNode> Lacing::MakeDecisionTree( const set<int> &possible_lacing_indices )
{    
    ASSERT( !possible_lacing_indices.empty() ); // Unexpected error: assert on best_cat should prevent this from failing
    
    // Deal with termination scenarios first: only one possible lacing index
    if( possible_lacing_indices.size()==1 )
    {
        // Get that index
        int index = OnlyElementOf( possible_lacing_indices );
        
        // Generate leaf node during unwind
        return make_shared<DecisionNodeLeaf>( index );
    }
    
    // Find the category whose lacing set best halves the set of possible lacing indices
    int target_inter_size = possible_lacing_indices.size() / 2;
    int best_diff = target_inter_size+1;
    TreePtr<Node> best_cat;  
    set<int> best_inter;  
    for( TreePtr<Node> cat : categories )
    {
        // size of intersection between current lacing set and possible indices
        set<int> inter = IntersectionOf( possible_lacing_indices,
                                         cats_to_lacing_sets.at(cat) );
                     
        // Skip where cat didn't provide any useful split of possible indices
        if( inter.empty() || inter.size()==possible_lacing_indices.size() )
            continue;
                                         
        // Closer to the target is better
        int diff = abs( (int)inter.size() - target_inter_size );
        if( diff < best_diff )
        {
            best_diff = diff;
            best_cat = cat;
            best_inter = inter; 
        }                                                                               
    }
        
    ASSERT( best_cat )
          ("Problem with the lacing: no category can differentiate these possible indices:\n")
          (possible_lacing_indices)("\n");
        
    // We'll need a set of possible indices that are NOT in best lacing set
    set<int> best_setdiff = DifferenceOf( possible_lacing_indices,
                                          cats_to_lacing_sets.at(best_cat) );

    // Recurse twice on the two hopefully-nearly-halves of the possible set
    shared_ptr<DecisionNode> yes = MakeDecisionTree( best_inter );
    shared_ptr<DecisionNode> no = MakeDecisionTree( best_setdiff );
    
    // Generate a decision node that should decide based on IsLocalMatch
    // during the unwind.
    return make_shared<DecisionNodeLocalMatch>( best_cat, yes, no );    
}


Lacing::DecisionNode::~DecisionNode()
{
}


Lacing::DecisionNodeLocalMatch::DecisionNodeLocalMatch( TreePtr<Node> category_, 
                                                        shared_ptr<DecisionNode> if_yes_,         
                                                        shared_ptr<DecisionNode> if_no_ ) :
    category( category_ ),
    if_yes( if_yes_ ),
    if_no( if_no_ )
{
}
    

int Lacing::DecisionNodeLocalMatch::GetLacingIndex( TreePtr<Node> candidate ) const
{
    return category->IsLocalMatch( candidate.get() ) ?
           if_yes->GetLacingIndex( candidate ) :
           if_no->GetLacingIndex( candidate );
}


string Lacing::DecisionNodeLocalMatch::Render(string pre)
{
    string s = "Are you a kind of " + Trace(category) + "?\n";
    s += pre + "✔ ⇒ " + if_yes->Render(pre+"    ");
    s += pre + "✘ ⇒ " + if_no ->Render(pre+"    ");
    return s;
}
        

Lacing::DecisionNodeLeaf::DecisionNodeLeaf( int lacing_index_ ) :
    lacing_index( lacing_index_ )
{
}


int Lacing::DecisionNodeLeaf::GetLacingIndex( TreePtr<Node> candidate ) const
{
    return lacing_index;
}


string Lacing::DecisionNodeLeaf::Render(string pre)
{
    return SSPrintf("Your lacing index is %d\n", lacing_index );
}
        
