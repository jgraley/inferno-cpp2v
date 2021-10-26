#include "cannonicaliser.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/duplicate.hpp"

#include <random>

#define CHECK_SC_AT_ROOT

using namespace SR;

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
        TreePtr<Node> child = (TreePtr<Node>)*it;
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
