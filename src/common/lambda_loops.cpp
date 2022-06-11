#include "lambda_loops.hpp"

#include "standard.hpp"
#include "trace.hpp"

#include <string>

bool IncrementIndices( vector<int> &indices, int index_range )
{
    for( int &i : indices )
    {
        i++;
        if( i==index_range )
            i = 0; // need to carry so iterate again
        else
            return false; // OK - did not wrap
    }
    return true; // wrapped
}


const vector<bool> index_range_bool = {false, true};

