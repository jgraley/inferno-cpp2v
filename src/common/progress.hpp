#ifndef PROGRESS_HPP
#define PROGRESS_HPP

#include <map>
#include <string>

using namespace std;

class Progress
{
public:
    enum Stage
    {
        INVALID, 
        BUILDING_STEPS,
        PLANNING_ONE,
        PLANNING_TWO,
        PARSING, 
        TRANSFORMING,
        RENDERING
    };
    
    Progress( Stage stage=INVALID, int step=-1 );
    string GetPrefix( int width=0 ) const;
    void SetAsCurrent() const;
    static const Progress &GetCurrent();
    inline bool operator!=( const Progress &o )
    {
        return stage != o.stage || step != o.step;
    }
    inline bool operator<( const Progress &o )
    {
        if( stage != o.stage )
            return (int)stage < (int)o.stage;
        else
            return step < o.step;
    }

private:    
    Stage stage;
    int step;

    static const map< Stage, pair<string, bool> > stage_formats;
    static Progress current;
};

#endif
