#include "relation_test.hpp"

#include <random>
 
using namespace SR;

#define RANDVAL_RANGE 1000000

void SR::TestRelationProperties( const unordered_set<XLink> &xlinks,
                                 bool expect_totality,
                                 string relation_name, 
                                 function<string()> log_on_fail,
                                 function<Orderable::Diff(XLink l, XLink r)> compare,
                                 function<bool(XLink l, XLink r)> is_equal_native, 
                                 function<XLink(XLink x, int randval)> get_special ) 
{
    // Measure the coverage    
    static map<string, int> tstab;
    static int tr=0, tlinks=0, tspecial=0;
    static map<string, int> ts;
    static map<string, map<string, int>> tt;     
    static map<string, int> ttot;

    // Need a random access container because we will in fact randomly access it
    vector<XLink> vxlinks;
    
    for( XLink xlink : xlinks )
        vxlinks.push_back(xlink);

    std::mt19937 random_gen;  // everyone's favourite engine: fast, long period
    std::uniform_int_distribution<int> random_index(0, vxlinks.size()-1);  // numbers in the range [0, vxlinks.size())
    std::uniform_int_distribution<int> random_special(0, RANDVAL_RANGE*2-1);  // numbers in the range [0, RANDVAL_RANGE*2)
    static const unsigned long int seed = 0;
    random_gen.seed(seed);
    auto random_xlink = [&]() -> XLink
    {        
        tlinks++;
        // should be a 50% chance so about half the x we test with are special
        int randval = random_special(random_gen);
        if( get_special && randval < RANDVAL_RANGE ) // Try to return special
        {
            XLink new_xlink, xlink;
            for( int t=0; t<vxlinks.size(); t++ ) // Only make this many attempts
            {
                xlink = vxlinks[random_index(random_gen)];
                new_xlink = get_special( xlink, randval );       
                if( new_xlink != xlink ) // Success
                {
                    tspecial++;
                    return new_xlink;            
                } 
            }
            // Ran out of tries for special
        }
        // Return a link from the supplied set
        return vxlinks[random_index(random_gen)];            
    };        

    // Stability property
    for( int i=0; i<vxlinks.size()*10; i++ )
    {
        XLink a_xlink = random_xlink();
        XLink b_xlink = random_xlink();
        Orderable::Diff ab_diff = compare(a_xlink, b_xlink);
        bool ab_eq_native = is_equal_native(a_xlink, b_xlink);
        if( ab_eq_native ) // Natively equal
        {
            ASSERT( ab_diff == 0)
                  (log_on_fail())("\n")
                  (relation_name)(" failed stability:\n")(a_xlink)(" ")(b_xlink);
            tstab["a !equiv b"]++;
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
                ASSERT( ab_diff != 0)
                      (log_on_fail())("\n")
                      (relation_name)(" failed totality:\n")
                      (a_xlink)(" != ")(b_xlink);
                ttot["a!=b"]++;
            }
        }
    }

    // Reflexive property
    for( XLink a_xlink : vxlinks )
    {
        Orderable::Diff aa_diff = compare(a_xlink, a_xlink);
        ASSERT( aa_diff == 0 )
              (log_on_fail())("\n")
              (relation_name)(" failed reflexivity:\n")
              (a_xlink);
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
            ASSERT( ba_diff == 0 )
                  (log_on_fail())("\n")
                  (relation_name)(" failed symmetry:\n")
                  (a_xlink)(" equiv ")(b_xlink);       
            ts["a equiv b"]++;
        }
        else if( ab_diff < 0 )        // a < b
        {
            ASSERT( ba_diff > 0)
                  (log_on_fail())("\n")
                  (relation_name)(" failed antisymmetry:\n")
                  (a_xlink)(" < ")(b_xlink);
            ts["a<b"]++;
        }
        else if( ab_diff > 0 )        // a > b
        {
            ASSERT( ba_diff < 0)
                  (log_on_fail())("\n")
                  (relation_name)(" failed antisymmetry:\n")
                  (a_xlink)(" > ")(b_xlink);
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
                ASSERT( ac_diff == 0 )
                      (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_xlink)(" equiv ")(b_xlink)(" equiv ")(c_xlink);
                t["b equiv c"]++;
            }
            else if( bc_diff < 0 )        // b < c
            {
                ASSERT( ac_diff < 0 )
                      (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_xlink)(" equiv ")(b_xlink)(" < ")(c_xlink);
                t["b<c"]++;
            }
            else if( bc_diff > 0 )        // b > c
            {
                ASSERT( ac_diff > 0 )
                      (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_xlink)(" equiv ")(b_xlink)(" > ")(c_xlink);
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
                      (a_xlink)(" < ")(b_xlink)(" equiv ")(c_xlink);
                t["b equiv c"]++;
            }
            else if( bc_diff < 0 )        // b < c
            {
                ASSERT( ac_diff < 0 )
					  (log_on_fail())("\n")
                      (relation_name)(" failed transitivity:\n")
                      (a_xlink)(" < ")(b_xlink)(" < ")(c_xlink);
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
                      (a_xlink)(" ")(b_xlink)(" ")(c_xlink);
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
                      (a_xlink)(" ")(b_xlink)(" ")(c_xlink);
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
         ("Made %d special of %d total XLinks\n", tspecial, tlinks)
         ("Stability ")(tstab)("\n")
         ("Reflexive ")(tr)("\n")
         ("Symmetric/antisymmetric ")(ts)("\n")
         ("Transitive ")(tt)("\n")
         ("Totality ")(ttot)("\n");
}

