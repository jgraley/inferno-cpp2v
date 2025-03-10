#ifndef RELATION_TEST_HPP
#define RELATION_TEST_HPP

#include "common/common.hpp"
#include "common/lambda_loops.hpp"
#include "node/specialise_oostd.hpp"
#include "../link.hpp"

#include <functional>

namespace SR
{

// K is probably an element of a set so it's its own key.
template<class K>
const K &GetKey( const K &x ) { return x; }

// K is probably an element of a map represented as a key-value pair
// (as seen in loops) so key is "first" of that pair.
template<class K, class V>
const K &GetKey( const pair<K, V> &x ) { return x.first; }

template<typename KeyType>
void TestRelationProperties( const vector<KeyType> &xlinks,
                             bool expect_totality,
                             string relation_name, 
                             function<Orderable::Diff(KeyType l, KeyType r)> compare,
                             function<bool(KeyType l, KeyType r)> is_equal_native, 
				     		 function<KeyType(KeyType x, int randval)> get_special = function<KeyType(KeyType x, int randval)>() );

template<class ORDERING>
void TestOrderingIntact( const ORDERING &ordering,
                         bool expect_totality,
                         string ordering_name )
{
    typedef typename ORDERING::value_type value_type;
    auto comp_func = ordering.key_comp();
    int t=0; 

    ForOverlappingAdjacentPairs( ordering, [&](const value_type &pred, 
                                               const value_type &succ)
    {
        // Test the STL interface to the relation, which is what ORDERING will actually use
        ASSERT( comp_func(GetKey(succ), GetKey(pred))==false )
              (ordering_name)(" failed broken ordering:\n")
              (pred)(" < ")(succ);
        if( expect_totality )
            ASSERT( comp_func(GetKey(pred), GetKey(succ))==true ) // antisymmetry and totality
                  (ordering_name)(" failed broken ordering:\n")
                  (succ)(" < ")(pred);
                    
        // To test using Compare3Way we'd have to construct our own instance
        // of the relation class.
        t++;
    } );

    TRACE("Ordering intactness tests for ")(ordering_name)(" passed. Coverage:\n")
         ("Pairs checked %d\n", t);
}

};

#endif
