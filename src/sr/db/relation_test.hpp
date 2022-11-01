#ifndef RELATION_TEST_HPP
#define RELATION_TEST_HPP

#include "common/common.hpp"
#include "common/lambda_loops.hpp"
#include "node/specialise_oostd.hpp"
#include "../link.hpp"

#include <functional>

namespace SR
{

void TestRelationProperties( const unordered_set<XLink> &xlinks,
                             bool expect_totality,
                             string relation_name, 
                             function<string()> log_on_fail,
                             function<Orderable::Diff(XLink l, XLink r)> compare,
                             function<bool(XLink l, XLink r)> is_equal_native, 
				     		 function<XLink(XLink x, int randval)> get_special = function<XLink(XLink x, int randval)>() );

template<class ORDERING>
void TestOrderingIntact( const ORDERING &ordering,
                         bool expect_totality,
                         string ordering_name )
{
    auto comp_func = ordering.key_comp();
    int t=0; 

    ForOverlappingAdjacentPairs( ordering, [&](const XLink &first, 
                                               const XLink &second)
    {
        // Test the STL interface to the relation, which is what CONTAINER will actually use
        ASSERT( comp_func(second, first)==false )
              (ordering_name)(" failed broken ordering:\n")
              (first)(" < ")(second);
        if( expect_totality )
            ASSERT( comp_func(first, second)==true ) // antisymmetry and totality
                  (ordering_name)(" failed broken ordering:\n")
                  (second)(" < ")(first);
                    
        // To test using Compare3Way we'd have to construct our own instance
        // of the relation class.
        t++;
    } );

    FTRACE("Ordering intactness tests for ")(ordering_name)(" passed. Coverage:\n")
          ("Pairs checked %d\n", t);
}

};

#endif
