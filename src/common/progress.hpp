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
    static constexpr int NO_STEP = -1;
    
    Progress( Stage stage=INVALID, int step=NO_STEP );
    Progress( string s );
    string GetPrefix( int width=0 ) const;
    int GetStep() const;
    Stage GetStage() const;
    bool IsValid() const;
    void SetAsCurrent() const;
    static const Progress &GetCurrent();
    static Progress GetCurrentStage();
    inline bool operator==( const Progress &o ) const 
    {
        return stage == o.stage && step == o.step;
    }
    inline bool operator!=( const Progress &o ) const 
    {
        return stage != o.stage || step != o.step;
    }
    inline bool operator<( const Progress &o ) const
    {
        if( stage != o.stage )
            return (int)stage < (int)o.stage;
        else
            return step < o.step;
    }

private:    
    Stage stage;
    int step;

    static const map<Stage, string> stage_formats;
    static Progress current;
};

#endif
