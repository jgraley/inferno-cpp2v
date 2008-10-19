#ifndef COMMON_HPP
#define COMMON_HPP

#include <assert.h>
#include <stdio.h>
#include <vector>
using namespace std;

#include <boost/shared_ptr.hpp>
using namespace boost;

// As suggested on http://www.boost.org/doc/libs/1_36_0/doc/html/foreach.html
#define foreach         BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

//#undef assert
//#define assert(X) do{ if(!X) (*(int *)-1)++; } while(0)

#include "trace.hpp"

#endif
