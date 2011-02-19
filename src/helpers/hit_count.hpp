#include <string>
#include "common/common.hpp"

class HitCount
{
public:    
    struct Category
    {
        string file;     // source file the HIT is in
        string function; // function the HIT is in
        unsigned line;   // line the HIT is on
        unsigned step;   // current step number
        void *instance;  // "this" pointer of the function, differentiates between master and slaves
        
    };
    
private:
    Map<Category, unsigned> counter;
    typedef pair<Category, unsigned> pc;
    unsigned current_step;

public:    
    void Hit( string file, string function, unsigned line, void *caller_this )
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
    
    void Dump()
    {
        FOREACH( pc p, counter )
            printf("#%3d @%p %s:%d in %s: %u\n", 
                   p.first.step,
                   p.first.instance,
                   p.first.file.c_str(),
                   p.first.line,
                   p.first.function.c_str(),
                   p.second );                   
    }
    
    void SetStep( int i )
    {
        current_step = i;
    }    
};

extern bool operator<( const HitCount::Category &l, const HitCount::Category &r );

#define HIT HitCount::instance.Hit( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, this )

