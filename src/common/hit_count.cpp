#include "hit_count.hpp"
#include "read_args.hpp"

HitCount HitCount::instance; 

#define HCM_FUNCTION 0x1
#define HCM_LINE     0x2
#define HCM_FILE     0x4
#define HCM_INSTANCE 0x8
#define HCM_STEP     0x10

bool operator<( const HitCount::Category &l, const HitCount::Category &r )
{
    // prioritise the comparisons in a way that makes for a nice dump when dumped in order
    if( (ReadArgs::hits_mask&HCM_STEP)==0 && l.step != r.step )
        return l.step < r.step;
    if( (ReadArgs::hits_mask&HCM_INSTANCE)==0 && l.instance != r.instance )
        return l.instance < r.instance;
    if( (ReadArgs::hits_mask&HCM_FILE)==0 && l.file != r.file )
        return l.file < r.file;
    if( (ReadArgs::hits_mask&HCM_LINE)==0 && l.line != r.line )
        return l.line < r.line;
    if( (ReadArgs::hits_mask&HCM_FUNCTION)==0 && l.function != r.function )
        return l.function < r.function;
    return false;
}

void HitCount::Dump()
{
    FOREACH( pc p, counter )
    {
        if( (ReadArgs::hits_mask&HCM_STEP)==0 ) 
            printf("#%3d ", p.first.step );                   
        if( (ReadArgs::hits_mask&HCM_INSTANCE)==0 ) 
            printf("@%p ", p.first.instance );                   
        if( (ReadArgs::hits_mask&HCM_FILE)==0 ) 
            printf("%s%s", p.first.file.c_str(), (ReadArgs::hits_mask&HCM_LINE)?" ":"" );
        if( (ReadArgs::hits_mask&HCM_LINE)==0 ) 
            printf(":%d ", p.first.line );                   
        if( (ReadArgs::hits_mask&HCM_FUNCTION)==0 ) 
            printf("in %s", p.first.function.c_str() );                   
        printf(": %u\n", p.second );  
    }                 
}

