#include "relation_test.hpp"

#include <random>
 
using namespace SR;

void SR::TestRelationProperties( function<Orderable::Diff(XLink l, XLink r)> compare,
                                 function<bool(XLink l, XLink r)> is_equal_native, 
                                 const unordered_set<XLink> &xlinks,
                                 bool expect_totality,
                                 string relation_name ) 
{
    // Need a random access container because we will in fact randomly access it
    vector<XLink> vxlinks;
    
    for( XLink xlink : xlinks )
        vxlinks.push_back(xlink);

    std::mt19937 random_gen;  // everyone's favourite engine: fast, long period
    std::uniform_int_distribution<int> random_index(0, vxlinks.size()-1);  // numbers in the range [0, vxlinks.size())
    static const unsigned long int seed = 0;
    random_gen.seed(seed);
    auto random_xlink = [&]()
    {
        return vxlinks[random_index(random_gen)];
    };
        
    // Measure the coverage    
    static int tr=0;
    static map<string, int> ts;
    static map<string, map<string, int>> tt;     
    static map<string, int> ttot;

    // Stability property
    for( int i=0; i<vxlinks.size()*10; i++ )
    {
        XLink a_xlink = random_xlink();
        XLink b_xlink = random_xlink();
        Orderable::Diff ab_diff = compare(a_xlink, b_xlink);
        bool ab_eq_native = is_equal_native(a_xlink, b_xlink);
        if( ab_eq_native ) // Natively equal
        {
            ASSERT( ab_diff == 0)(relation_name)(" failed stability:\n")(a_xlink)(" ")(b_xlink);
            ts["a!=b"]++;
        }
    }

    // Reflexive property
    for( XLink a_xlink : vxlinks )
    {
        Orderable::Diff aa_diff = compare(a_xlink, a_xlink);
        ASSERT( aa_diff == 0 )(relation_name)(" failed reflexivity:\n")(a_xlink);
        tr++;
    }
    
    // Symmetric/antisymmetric property
    for( int i=0; i<vxlinks.size()*10; i++ )
    {
        XLink a_xlink = random_xlink();
        XLink b_xlink = random_xlink();
        Orderable::Diff ab_diff = compare(a_xlink, b_xlink);
        Orderable::Diff ba_diff = compare(b_xlink, a_xlink);
        if( ab_diff == 0 )            // a == b
        {
            ASSERT( ba_diff == 0 )(relation_name)(" failed symmetry:\n")(a_xlink)(" ")(b_xlink);       
            ts["a==b"]++;
        }
        else if( ab_diff < 0 )        // a < b
        {
            ASSERT( ba_diff > 0)(relation_name)(" failed antisymmetry:\n")(a_xlink)(" ")(b_xlink);
            ts["a<b"]++;
        }
        else if( ab_diff > 0 )        // a > b
        {
            ASSERT( ba_diff < 0)(relation_name)(" failed antisymmetry:\n")(a_xlink)(" ")(b_xlink);
            ts["a>b"]++;
        }
        else
        {
            ASSERTFAIL("huh?\n");            
        }
    }
     
    // Transitive property
    for( int i=0; i<vxlinks.size()*10; i++ )
    {
        XLink a_xlink = random_xlink();
        XLink b_xlink = random_xlink();
        XLink c_xlink = random_xlink();
        Orderable::Diff ab_diff = compare(a_xlink, b_xlink);
        Orderable::Diff bc_diff = compare(b_xlink, c_xlink);
        Orderable::Diff ac_diff = compare(a_xlink, c_xlink);
        if( ab_diff == 0 )            // a == b
        {
			map<string, int> &t = tt["a==b"];
            if( bc_diff == 0 )            // b == c
            {
                ASSERT( ac_diff == 0 )(relation_name)(" failed transitivity:\n")(a_xlink)(" ")(b_xlink)(" ")(c_xlink);
                t["b==c"]++;
            }
            else if( bc_diff < 0 )        // b < c
            {
                ASSERT( ac_diff < 0 )(relation_name)(" failed transitivity:\n")(a_xlink)(" ")(b_xlink)(" ")(c_xlink);
                t["b<c"]++;
            }
            else if( bc_diff > 0 )        // b > c
            {
                ASSERT( ac_diff > 0 )(relation_name)(" failed transitivity:\n")(a_xlink)(" ")(b_xlink)(" ")(c_xlink);
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
                ASSERT( ac_diff < 0 )(relation_name)(" failed transitivity:\n")(a_xlink)(" ")(b_xlink)(" ")(c_xlink);
                t["b==c"]++;
            }
            else if( bc_diff < 0 )        // b < c
            {
                ASSERT( ac_diff < 0 )(relation_name)(" failed transitivity:\n")(a_xlink)(" ")(b_xlink)(" ")(c_xlink);
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
                ASSERT( ac_diff > 0 )(relation_name)(" failed transitivity:\n")(a_xlink)(" ")(b_xlink)(" ")(c_xlink);
                t["b==c"]++;
            }
            else if( bc_diff < 0 )        // b < c
            {
                // no information relating a to c
                t["b<c"]++;
            }
            else if( bc_diff > 0 )        // b > c
            {
                ASSERT( ac_diff > 0 )(relation_name)(" failed transitivity:\n")(a_xlink)(" ")(b_xlink)(" ")(c_xlink);
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
    
    if( expect_totality )
    {
        // Totality property
        for( int i=0; i<vxlinks.size()*10; i++ )
        {
            XLink a_xlink = random_xlink();
            XLink b_xlink = random_xlink();
            Orderable::Diff ab_diff = compare(a_xlink, b_xlink);
            bool ab_eq_native = is_equal_native(a_xlink, b_xlink);
            if( !ab_eq_native ) // Natively not equal
            {
                ASSERT( ab_diff != 0)(relation_name)(" failed totality:\n")(a_xlink)(" ")(b_xlink);
                ts["a!=b"]++;
            }
        }
    }
    
    // See #210 for some results
    FTRACE("Relation tests passed. Coverage:\n")
          ("reflexive ")(tr)("\n")
          ("symmetric/antisymmetric ")(ts)("\n")
          ("transitive ")(tt)("\n")
          ("totality ")(tt)("\n");
}

