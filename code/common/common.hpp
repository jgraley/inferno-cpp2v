#ifndef COMMON_HPP
#define COMMON_HPP

#include <assert.h>
#include <stdio.h>
#include <vector>
using namespace std;

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
using namespace boost;

// As suggested on http://www.boost.org/doc/libs/1_36_0/doc/html/foreach.html
#define foreach         BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

#define COUNTOF( array ) ( sizeof( array )/sizeof( array[0] ) )

#include "trace.hpp"

#endif
