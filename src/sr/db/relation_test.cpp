#include "relation_test.hpp"

#include <random>
 
using namespace SR;

void SR::TestRelationProperties( function<Orderable::Diff(XLink l, XLink r)> compare,
                                 const unordered_set<XLink> &xlinks ) 
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
    static vector<int> ts(3);
    static vector<int> tt(9);     
        
    // Reflexive property
    for( XLink a_xlink : vxlinks )
    {
        Orderable::Diff aa_cr = compare(a_xlink, a_xlink);
        ASSERT( aa_cr == 0 );
        tr++;
    }
    
    // Symmetric/antisymmetric property
    for( int i=0; i<vxlinks.size()*10; i++ )
    {
        XLink a_xlink = random_xlink();
        XLink b_xlink = random_xlink();
        Orderable::Diff ab_cr = compare(a_xlink, b_xlink);
        Orderable::Diff ba_cr = compare(b_xlink, a_xlink);
        if( ab_cr == 0 )            // a == b
        {
            ASSERT( ba_cr == 0 );       
            ts[0]++;
        }
        else if( ab_cr < 0 )        // a < b
        {
            ASSERT( ba_cr > 0);
            ts[1]++;
        }
        else if( ab_cr > 0 )        // a > b
        {
            ASSERT( ba_cr < 0);
            ts[2]++;
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
        Orderable::Diff ab_cr = compare(a_xlink, b_xlink);
        Orderable::Diff bc_cr = compare(b_xlink, c_xlink);
        Orderable::Diff ac_cr = compare(a_xlink, c_xlink);
        if( ab_cr == 0 )            // a == b
        {
            if( bc_cr == 0 )            // b == c
            {
                ASSERT( ac_cr == 0 );
                tt[0]++;
            }
            else if( bc_cr < 0 )        // b < c
            {
                ASSERT( ac_cr < 0 );
                tt[1]++;
            }
            else if( bc_cr > 0 )        // b > c
            {
                ASSERT( ac_cr > 0 );
                tt[2]++;
            }
            else
            {
                ASSERTFAIL("huh?\n");
            }
        }
        else if( ab_cr < 0 )        // a < b
        {
            if( bc_cr == 0 )            // b == c
            {
                ASSERT( ac_cr < 0 );
                tt[3]++;
            }
            else if( bc_cr < 0 )        // b < c
            {
                ASSERT( ac_cr < 0 );
                tt[4]++;
            }
            else if( bc_cr > 0 )        // b > c
            {
                // no information relating a to c
            }
            else
            {
                ASSERTFAIL("huh?\n");
            }
        }
        else if( ab_cr > 0 )        // a > b
        {
            if( bc_cr == 0 )            // b == c
            {
                ASSERT( ac_cr > 0 );
                tt[6]++;
            }
            else if( bc_cr < 0 )        // b < c
            {
                // no information relating a to c
            }
            else if( bc_cr > 0 )        // b > c
            {
                ASSERT( ac_cr > 0 );
                tt[8]++;
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
    FTRACE("Relation tests passed. Coverage:\n")
          ("reflexive ")(tr)("\n")
          ("symmetric/antisymmetric ")(ts)("\n")
          ("transitive ")(tt)("\n");
}

