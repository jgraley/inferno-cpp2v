#ifndef HIT_COUNT_H
#define HIT_COUNT_H

#include <string>
#include <map>
#include "standard.hpp"

class HitCount
{
public:    
    struct Category
    {
        string file;     // source file the HIT is in
        string function; // function the HIT is in
        unsigned line;   // line the HIT is on
        unsigned step;   // current step number
        const void *instance;  // "this" pointer of the function, differentiates between master and slaves        
    };
    
private:
    Map<Category, unsigned> counter;
    typedef pair<Category, unsigned> pc;
    unsigned current_step;
    static bool enable;
    
public:    
    static HitCount instance;
    void Hit( string file, unsigned line, string function, const void *caller_this )
    {
        Category c;
        c.file = file;
        c.function = function;
        c.line = line;
        c.step = current_step;
        c.instance = caller_this;        
        int count=0;
        if( counter.IsExist( c ) )        
            count = counter[c];
        count++;
        counter[c] = count;            
    }
    
    void Dump();
    
    void SetStep( int i )
    {
        current_step = i;
    }    
    
    static void Enable( bool e ); ///< enable/disable hit counting, only for top level function to call, overridden by flags
    inline static bool IsEnabled() { return enable; }    
};

extern bool operator<( const HitCount::Category &l, const HitCount::Category &r );

#define HIT if(HitCount::IsEnabled()) HitCount::instance.Hit( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, this )

#endif

