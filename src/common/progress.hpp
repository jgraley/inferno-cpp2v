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
        PATTERN_TRANS,
        PLANNING_ONE,
        PLANNING_TWO,
        PLANNING_THREE,
        PLANNING_FOUR,
        PLANNING_FIVE, // See #545
        PARSING, // not a VN stage
        ANALYSING,
        TRANSFORMING,
        RENDERING // not a VN stage
    };
    
    enum Steppiness
    {
        STEPPY,
        NON_STEPPY
    };
    
    static constexpr int NO_STEP = -1;
    
    Progress( Stage stage=INVALID, int step=NO_STEP );
    Progress( string s );
    string GetPrefix( int width=0 ) const;
    int GetStep() const;
    Stage GetStage() const;
    Steppiness GetSteppiness() const;
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
    struct StageInfoBlock
    {
        string code;
        Steppiness steppiness;
    };

    static const map<Stage, StageInfoBlock> stage_info;
    static Progress current;
};

#endif
