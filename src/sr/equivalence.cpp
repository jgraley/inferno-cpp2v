#include "equivalence.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/duplicate.hpp"

#include <random>

using namespace SR;

//////////////////////////// EquivalenceRelation ///////////////////////////////

EquivalenceRelation::EquivalenceRelation() :
    simple_compare( make_shared<SimpleCompare>() )
{
}


CompareResult EquivalenceRelation::Compare( XLink xlink, XLink ylink ) const
{
    // Get the child nodes and disregard the arrow heads
    TreePtr<Node> xnode = xlink.GetChildX();
    TreePtr<Node> ynode = ylink.GetChildX();
    
    // And then resort to SimpleCompare
    return simple_compare->Compare( xnode, ynode );
}


bool EquivalenceRelation::operator()( XLink xlink, XLink ylink ) const
{
    return Compare(xlink, ylink) < EQUAL;
}


void EquivalenceRelation::TestProperties( const set<XLink> &xlinks ) const
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
        CompareResult aa_cr = Compare(a_xlink, a_xlink);
        ASSERT( aa_cr == EQUAL );
        tr++;
    }
    
    // Symmetric/antisymmetric property
    for( int i=0; i<vxlinks.size()*10; i++ )
    {
        XLink a_xlink = random_xlink();
        XLink b_xlink = random_xlink();
        CompareResult ab_cr = Compare(a_xlink, b_xlink);
        CompareResult ba_cr = Compare(b_xlink, a_xlink);
        if( ab_cr == EQUAL )            // a == b
        {
            ASSERT( ba_cr == EQUAL );       
            ts[0]++;
        }
        else if( ab_cr < EQUAL )        // a < b
        {
            ASSERT( ba_cr > EQUAL);
            ts[1]++;
        }
        else if( ab_cr > EQUAL )        // a > b
        {
            ASSERT( ba_cr < EQUAL);
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
        CompareResult ab_cr = Compare(a_xlink, b_xlink);
        CompareResult bc_cr = Compare(b_xlink, c_xlink);
        CompareResult ac_cr = Compare(a_xlink, c_xlink);
        if( ab_cr == EQUAL )            // a == b
        {
            if( bc_cr == EQUAL )            // b == c
            {
                ASSERT( ac_cr == EQUAL );
                tt[0]++;
            }
            else if( bc_cr < EQUAL )        // b < c
            {
                ASSERT( ac_cr < EQUAL );
                tt[1]++;
            }
            else if( bc_cr > EQUAL )        // b > c
            {
                ASSERT( ac_cr > EQUAL );
                tt[2]++;
            }
            else
            {
                ASSERTFAIL("huh?\n");
            }
        }
        else if( ab_cr < EQUAL )        // a < b
        {
            if( bc_cr == EQUAL )            // b == c
            {
                ASSERT( ac_cr < EQUAL );
                tt[3]++;
            }
            else if( bc_cr < EQUAL )        // b < c
            {
                ASSERT( ac_cr < EQUAL );
                tt[4]++;
            }
            else if( bc_cr > EQUAL )        // b > c
            {
                // no information relating a to c
            }
            else
            {
                ASSERTFAIL("huh?\n");
            }
        }
        else if( ab_cr > EQUAL )        // a > b
        {
            if( bc_cr == EQUAL )            // b == c
            {
                ASSERT( ac_cr > EQUAL );
                tt[6]++;
            }
            else if( bc_cr < EQUAL )        // b < c
            {
                // no information relating a to c
            }
            else if( bc_cr > EQUAL )        // b > c
            {
                ASSERT( ac_cr > EQUAL );
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

//////////////////////////// QuotientSet ///////////////////////////////

XLink QuotientSet::Uniquify( XLink xlink )
{
    // insert() only acts if element not already in set.
    // Conveniently, it returns an iterator to the matching element
    // regardless of whether x was inserted, so it's always what we
    // want to return. p.second is true if insertion took place, useful 
    // for tracing etc.
    pair<Classes::iterator, bool> p = classes.insert( xlink );
    return *p.first;
}


void QuotientSet::Clear()
{
    classes.clear();
}
