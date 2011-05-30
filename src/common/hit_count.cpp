#include "hit_count.hpp"

bool operator<( const HitCount::Category &l, const HitCount::Category &r )
{
    // prioritise the comparisons in a way that makes for a nice dump when dumped in order
    if( l.step != r.step )
        return l.step < r.step;
    if( l.instance != r.instance )
        return l.instance < r.instance;
    if( l.file != r.file )
        return l.file < r.file;
    if( l.function != r.function )
        return l.function < r.function;
    if( l.line != r.line )
        return l.line < r.line;
    return false;
}
