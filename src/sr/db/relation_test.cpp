#include "relation_test.hpp"

#include <random>
 
using namespace SR;

#define RANDVAL_RANGE 1000000


template<typename KeyType>
void SR::TestRelationProperties( const unordered_set<KeyType> &keys,
                                 bool expect_totality,
                                 string relation_name, 
                                 function<string()> log_on_fail,
                                 function<Orderable::Diff(KeyType l, KeyType r)> compare,
                                 function<bool(KeyType l, KeyType r)> is_equal_native, 
                                 function<KeyType(KeyType x, int randval)> get_special ) 
{
    // Measure the coverage    
    static map<string, int> tstab;
    static int tr=0, tkeys=0, tspecial=0;
    static map<string, int> ts;
    static map<string, map<string, int>> tt;     
    static map<string, int> ttot;

    // Need a random access container because we will in fact randomly access it
    vector<KeyType> vkeys;
    
    for( KeyType key : keys )
        vkeys.push_back(key);

    std::mt19937 random_gen;  // everyone's favourite engine: fast, long period
    std::uniform_int_distribution<int> random_index(0, vkeys.size()-1);  // numbers in the range [0, vkeys.size())
    std::uniform_int_distribution<int> random_special(0, RANDVAL_RANGE*2-1);  // numbers in the range [0, RANDVAL_RANGE*2)
    static const unsigned long int seed = 0;
    random_gen.seed(seed);
    auto get_random_key = [&]() -> KeyType
    {        
        tkeys++;
        // should be a 50% chance so about half the x we test with are special
        int randval = random_special(random_gen);
        if( get_special && randval < RANDVAL_RANGE ) // Try to return special
        {
            KeyType new_key, key;
            for( int t=0; t<vkeys.size(); t++ ) // Only make this many attempts
            {
                key = vkeys[random_index(random_gen)];
                new_key = get_special( key, randval );       
                if( new_key != key ) // Success
                {
                    tspecial++;
                    return new_key;            
                } 
            }
            // Ran out of tries for special
        }
        // Return a link from the supplied set
        return vkeys[random_index(random_gen)];            
    };        

    // Stability property
    for( int i=0; i<vkeys.size()*10; i++ )
    {
        KeyType a_key = get_random_key();
        KeyType b_key = get_random_key();
        Orderable::Diff ab_diff = compare(a_key, b_key);
        bool ab_eq_native = is_equal_native(a_key, b_key);
        if( ab_eq_native ) // Natively equal
        {
            ASSERT( ab_diff == 0)
                  (log_on_fail())("\n")
                  (relation_name)(" failed stability:\n")(a_key)(" ")(b_key);
            tstab["a !equiv b"]++;
        }
    }

    if( expect_totality )
    {
        // Totality property
        for( int i=0; i<vkeys.size()*10; i++ )
        {
            KeyType a_key = get_random_key();
            KeyType b_key = get_random_key();
            Orderable::Diff ab_diff = compare(a_key, b_key);
            bool ab_eq_native = is_equal_native(a_key, b_key);
            if( !ab_eq_native ) // Natively not equal
            {
                ASSERT( ab_diff != 0)
                      (log_on_fail())("\n")
                      (relation_name)(" failed totality:\n")
                      (a_key)(" != ")(b_key);
                ttot["a!=b"]++;
            }
        }
    }

    // Reflexive property
    for( KeyType a_key : vkeys )
    {
        Orderable::Diff aa_diff = compare(a_key, a_key);
        ASSERT( aa_diff == 0 )
              (log_on_fail())("\n")
              (relation_name)(" failed reflexivity:\n")
              (a_key);
        tr++;
    }
    
    // Symmetric/antisymmetric property
    for( int i=0; i<vkeys.size()*10; i++ )
    {
        KeyType a_key = get_random_key();
        KeyType b_key = get_random_key();
        Orderable::Diff ab_diff = compare(a_key, b_key);
        Orderable::Diff ba_diff = compare(b_key, a_key);
        if( ab_diff == 0 )            // a == b
        {
            ASSERT( ba_diff == 0 )
                  (log_on_fail())("\n")
                  (relation_name)(" failed symmetry:\n")
                  (a_key)(" equiv ")(b_key);       
            ts["a equiv b"]++;
        }
        else if( ab_diff < 0 )        // a < b
        {
            ASSERT( ba_diff > 0)
                  (log_on_fail())("\n")
                  (relation_name)(" failed antisymmetry:\n")
                  (a_key)(" < ")(b_key);
            ts["a<b"]++;
        }
        else if( ab_diff > 0 )        // a > b
        {
            ASSERT( ba_diff < 0)
                  (log_on_fail())("\n")
                  (relation_name)(" failed antisymmetry:\n")
                  (a_key)(" > ")(b_key);
            ts["a>b"]++;
        }
        else
        {
            ASSERTFAIL("huh?\n");            
        }
    }
     
    // Transitive property
    for( int i=0; i<vkeys.size()*10; i++ )
    {
        KeyType a_key = get_random_key();
        KeyType b_key = get_random_key();
        KeyType c_key = get_random_key();
        Orderable::Diff ab_diff = compare(a_key, b_key);
        Orderable::Diff bc_diff = compare(b_key, c_key);
        Orderable::Diff ac_diff = compare(a_key, c_key);
        if( ab_diff == 0 )            // a == b
        {
			map<string, int> &t = tt["a==b"];
            if( bc_diff == 0 )            // b == c
            {
                ASSERT( ac_diff == 0 )
                      (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_key)(" equiv ")(b_key)(" equiv ")(c_key);
                t["b equiv c"]++;
            }
            else if( bc_diff < 0 )        // b < c
            {
                ASSERT( ac_diff < 0 )
                      (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_key)(" equiv ")(b_key)(" < ")(c_key);
                t["b<c"]++;
            }
            else if( bc_diff > 0 )        // b > c
            {
                ASSERT( ac_diff > 0 )
                      (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_key)(" equiv ")(b_key)(" > ")(c_key);
                t["b>c"]++;
            }
            else
            {
                ASSERTFAIL("huh?\n");
            }
        }
        else if( ab_diff < 0 )        // a < b
        {
			map<string, int> &t = tt["a<b"];
            t["b>c"];
            if( bc_diff == 0 )            // b == c
            {
                ASSERT( ac_diff < 0 )
                      (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_key)(" < ")(b_key)(" equiv ")(c_key);
                t["b equiv c"]++;
            }
            else if( bc_diff < 0 )        // b < c
            {
                ASSERT( ac_diff < 0 )
					  (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_key)(" < ")(b_key)(" < ")(c_key);
                t["b<c"]++;
            }
            else if( bc_diff > 0 )        // b > c
            {
                // no information relating a to c
                t["b>c"]++;
            }
            else
            {
                ASSERTFAIL("huh?\n");
            }
        }
        else if( ab_diff > 0 )        // a > b
        {
			map<string, int> &t = tt["a>b"];
            if( bc_diff == 0 )            // b == c
            {
                ASSERT( ac_diff > 0 )
                      (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_key)(" ")(b_key)(" ")(c_key);
                t["b equiv c"]++;
            }
            else if( bc_diff < 0 )        // b < c
            {
                // no information relating a to c
                t["b<c"]++;
            }
            else if( bc_diff > 0 )        // b > c
            {
                ASSERT( ac_diff > 0 )
                      (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_key)(" ")(b_key)(" ")(c_key);
                t["b>c"]++;
            }
            else
            {
                ASSERTFAIL("huh?\n");
            }
        }
        else
        {
            ASSERTFAIL("huh?\n");
        }
    }    
        
    // See #210 for some results
    TRACE("Relation tests for ")(relation_name)(" passed. Coverage:\n")
         ("Made %d special keys of %d total keys\n", tspecial, tkeys)
         ("Stability ")(tstab)("\n")
         ("Reflexive ")(tr)("\n")
         ("Symmetric/antisymmetric ")(ts)("\n")
         ("Transitive ")(tt)("\n")
         ("Totality ")(ttot)("\n");
}

// --------------------- Explicit instantiations for the types we use as keys --------------------------

template void SR::TestRelationProperties<XLink>( const unordered_set<XLink> &keys,
										         bool expect_totality,
										         string relation_name, 
										         function<string()> log_on_fail,
										         function<Orderable::Diff(XLink l, XLink r)> compare,
										         function<bool(XLink l, XLink r)> is_equal_native, 
										         function<XLink(XLink x, int randval)> get_special );

template void SR::TestRelationProperties<TreePtr<Node>>( const unordered_set<TreePtr<Node>> &keys,
														 bool expect_totality,
														 string relation_name, 
														 function<string()> log_on_fail,
														 function<Orderable::Diff(TreePtr<Node> l, TreePtr<Node> r)> compare,
														 function<bool(TreePtr<Node> l, TreePtr<Node> r)> is_equal_native, 
														 function<TreePtr<Node>(TreePtr<Node> x, int randval)> get_special );
