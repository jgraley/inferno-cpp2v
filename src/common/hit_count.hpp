#ifndef HIT_COUNT_H
#define HIT_COUNT_H

#include "standard.hpp"
#include "progress.hpp"

#include <string>
#include <map>


class HitCount
{
public:    
    struct Category
    {
        string file;     // source file the HIT is in
        string function; // function the HIT is in
        unsigned line;   // line the HIT is on
        Progress progress;   // current stage/step number
        const Traceable *instance;  // "this" pointer of the function, differentiates between master and slaves        
        string prefix;
    };
        
    void Hit( string file, unsigned line, string function, const Traceable *caller_this, string prefix="" )
    {
        Category c;
        c.file = file;
        c.function = function;
        c.line = line;
        c.progress = Progress::GetCurrent();
        c.instance = caller_this;        
        c.prefix = prefix;
        int count=0;
        if( counter.count( c ) > 0 )        
            count = counter[c];
        count++;
        counter[c] = count;            
    }
    
    void Usage();
    void Check();
    void Dump();

    static void Enable( bool e ); ///< enable/disable hit counting, only for top level function to call, overridden by flags
    inline static bool IsEnabled() { return enable; }    

    static HitCount instance;    
private:
    map<Category, unsigned> counter;
    typedef pair<Category, unsigned> pc;
    static bool enable;
};

extern bool operator<( const HitCount::Category &l, const HitCount::Category &r );

#define HIT do { if(HitCount::IsEnabled()) HitCount::instance.Hit( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, this ); } while(false)
#define HITP(P) do { if(HitCount::IsEnabled()) HitCount::instance.Hit( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, this, P ); } while(false)

#endif

