#include "equivalence.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/duplicate.hpp"

#include <random>

#define CHECK_SC_AT_ROOT

using namespace SR;

//////////////////////////// EquivalenceRelation ///////////////////////////////

EquivalenceRelation::EquivalenceRelation() :
    simple_compare( make_shared<SimpleCompare>() )
{
}


CompareResult EquivalenceRelation::Compare( XLink xlink, XLink ylink )
{
    // Get the child nodes and disregard the arrow heads
    TreePtr<Node> xnode = xlink.GetChildX();
    TreePtr<Node> ynode = ylink.GetChildX();
    
    // And then resort to SimpleCompare
    return simple_compare->Compare( xnode, ynode );
}


bool EquivalenceRelation::operator()( XLink xlink, XLink ylink )
{
    return Compare(xlink, ylink) < EQUAL;
}


void EquivalenceRelation::TestProperties( const set<XLink> &xlinks )
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
        
    static int tr=0;
    static vector<int> ts(3);
    static vector<int> tt(9);    
        
    // Reflexive
    for( XLink a_xlink : vxlinks )
    {
        CompareResult aa_cr = Compare(a_xlink, a_xlink);
        ASSERT( aa_cr == EQUAL );
        tr++;
    }
    
    // Symmetric/antisymmetric
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
            ASSERTFAIL("huh?\n");
    }
     
    // Transitive
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
                ASSERTFAIL("huh?\n");
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
                ASSERT( true ); // no information relating a to c
                tt[5]++;
            }
            else
                ASSERTFAIL("huh?\n");
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
                ASSERT( true ); // no information relating a to c
                tt[7]++;
            }
            else if( bc_cr > EQUAL )        // b > c
            {
                ASSERT( ac_cr > EQUAL );
                tt[8]++;
            }
            else
                ASSERTFAIL("huh?\n");
        }
        else
            ASSERTFAIL("huh?\n");
    }    
    
    FTRACE("Relation passed tests:\n")
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
    // want to return. p.second is true if insertion tooke place, useful 
    // for tracing etc.
    pair<Classes::iterator, bool> p = classes.insert( xlink );
    return *p.first;
}


void QuotientSet::Clear()
{
    classes.clear();
}

//////////////////////////// Cannonicaliser ///////////////////////////////

void Cannonicaliser::operator()( TreePtr<Node> context, 
       		                     TreePtr<Node> *proot )
{
#ifdef CHECK_SC_AT_ROOT
    TreePtr<Node> dup = Duplicate::DuplicateSubtree( *proot );
#endif

    // Do pass one!
    int max_shallowness = PassOneWalk( *proot );
    
    TRACE("parent_pointers: ")(parent_pointers)("\n")
         ("nodes_by_shallowness: ")(nodes_by_shallowness)("\n")
         ("max_shallowness: ")(max_shallowness)("\n");
    
    // Do pass two!
    uniques_by_shallowness.resize( max_shallowness+1 );
    for( int shallowness = 0; shallowness <= max_shallowness; shallowness++ )
    {
        TRACE("Uniquify shallowness ")(shallowness)("\n");
        Uniquify( nodes_by_shallowness[shallowness], 
                  uniques_by_shallowness[shallowness] );  
    }  
    
#ifdef CHECK_SC_AT_ROOT
    SimpleCompare sc;
    ASSERT( sc.Compare( *proot, dup ) == EQUAL );
#endif
}                                 

/*
 * Shallowness (sh) is max(generations to leaf node) and if sh(x) != sh(y)
 * then SimpleCompare(x, y) will be false, I think.
 */
int Cannonicaliser::PassOneWalk( TreePtr<Node> node )
{
    // Do the recursion part, populate parent pointers map and determine shallowness
    int shallowness = 0;
    FlattenNode f( node );
    for( FlattenNode::iterator it = f.begin(); it != f.end(); ++it )
    {        
        TreePtr<Node> child = *it;
        list< FlattenNode::iterator > &child_pp = parent_pointers[child];
        if( find( child_pp.begin(), child_pp.end(), it ) != child_pp.end() ) // TODO if this was a set this would be fast, but need < on all the iterators
            continue; // already reached here
            
        child_pp.push_back( it );
        shallowness = max(shallowness, PassOneWalk( child )+1 );
    }
        
    // Populate nodes by shallowness
    if( shallowness >= nodes_by_shallowness.size() )
        nodes_by_shallowness.resize( shallowness+1 );
    nodes_by_shallowness[shallowness].insert( node );
        
    return shallowness;
}


void Cannonicaliser::Uniquify( const set< TreePtr<Node> > &nodes, 
                               set< TreePtr<Node> > &uniques )
{
    SimpleCompare sc;
    
    for( TreePtr<Node> n : nodes )
    {
        TRACE("Looking for node ")(n)(" in uniques ")(uniques)("\n");
        TreePtr<Node> found_in_uniques;    
        for( TreePtr<Node> u : uniques )
        {
            if( sc.Compare(u, n) == EQUAL )
            {
                found_in_uniques = u;
                break;
            }
        }
        
        if( found_in_uniques )
        {
            TRACEC("Found: ")(found_in_uniques)(" - redirecting\n");
            // not unique, so replace with the unique one
            RedirectParentPointers( n, found_in_uniques );
        }
        else
        {
            TRACE("Not found - inserting\n");
            // unique, so remember it
            uniques.insert( n );
        }
    }
}


void Cannonicaliser::RedirectParentPointers( TreePtr<Node> from, TreePtr<Node> to )
{
    TRACE("Redirecting from ")(from)(" to ")(to)("\n");
    TRACEC("From-parent pointers: ")(parent_pointers[from])("\n");
    for( FlattenNode::iterator from_it : parent_pointers[from] )
    {
        ASSERT( *from_it == from )("Expected from-iterator ")(from_it)(" to dereference to ")(from)("\n");
        
        // Note that because pointers to children can be in Collections, which could
        // be implemented using eg std::set, we must hold FlattenNode iterators and
        // when the time comes to redirect a pointer, use Overwrite().
        TRACEC("Overwriting iterator ")(from_it)(" with ")(to)("\n");
        from_it.Overwrite( &to );
        
        parent_pointers[to].push_back( from_it );        
    }
    
    parent_pointers.erase(from);
}
