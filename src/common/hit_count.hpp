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
        Progress progress;   // current stage/step number
        string file;     // source file the HIT is in
        unsigned line;   // line the HIT is on
        string function; // function the HIT is in
        string instance;  // trace string representing the instance        
        string prefix;
    };
        
    void Hit( string file, unsigned line, string function, string instance, string prefix="" )
    {
        Category c;
        c.progress = Progress::GetCurrent();
        c.file = file;
        c.line = line;
        c.instance = instance;        
        c.function = function;
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

#define HIT do { if(HitCount::IsEnabled()) HitCount::instance.Hit( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, GetTrace() ); } while(false)
#define HITP(P) do { if(HitCount::IsEnabled()) HitCount::instance.Hit( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, GetTrace(), P ); } while(false)
#define HITS do { if(HitCount::IsEnabled()) HitCount::instance.Hit( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, "" ); } while(false)
#define HITSP(P) do { if(HitCount::IsEnabled()) HitCount::instance.Hit( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, "", P ); } while(false)

#endif

