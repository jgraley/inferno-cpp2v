#ifndef COMMON_HPP
#define COMMON_HPP

#include <assert.h>
#include <stdio.h>
#include <vector>
using namespace std;

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
using namespace boost;

// TODO use BOOST_FOREACH when cygwin boost gets past rev 1.33
// For now, use my rubbish version
#define FOREACH(M, C) \
    if( int __i=1 ) \
        for( bool __d = true; \
             __d && __i<=(C).size(); \
             __i++ ) \
            if( (__d=false) ) {} \
            else \
                for( M=(C)[__i-1]; !__d; __d=true ) 
  

#define COUNTOF( array ) ( sizeof( array )/sizeof( array[0] ) )

#include "trace.hpp"

#endif
