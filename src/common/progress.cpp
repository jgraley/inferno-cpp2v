#include "progress.hpp"

#include "standard.hpp"

#include <algorithm>

Progress::Progress( Stage stage_, int step_ ) :
    stage(stage_),
    step(step_)
{    
}


Progress::Progress( string s )
{    
    for( auto p : Progress::stage_formats )
    {
        stage = p.first;
        string stage_code = p.second;
        if( s.substr(0, stage_code.size()) != stage_code)
            continue;
            
        if( s.size() == stage_code.size() )
        {
            step = NO_STEP;
            return; // success: no step        
        }
            
        string ss = s.substr(stage_code.size());                
        step = atoi(ss.c_str());
        if( step==0 && ss != "0" )
            break; // error - invalid
            
        return; // done - got a step
    }
    
    // Didn't find stage
    stage = INVALID;
    step = -1;
}


string Progress::GetPrefix(int width) const
{
    if( stage==INVALID )
        return string( width, ' '); // defensive
        
    string stage_code = stage_formats.at(stage);
    int width_after_stage_code = max((string::size_type)0, width-stage_code.size());
    
    if( width==0 )
        return stage_code + (step!=NO_STEP ? to_string(step) : string());        
    else if( step!=NO_STEP )
        return stage_code + SSPrintf( "%0"+to_string(width_after_stage_code)+"d", step );
    else
        return stage_code + string( width_after_stage_code, ' ');
}


int Progress::GetStep() const
{
    return step;
}


Progress::Stage Progress::GetStage() const
{
    return stage;
}


bool Progress::IsValid() const
{
    return stage != INVALID;
}


void Progress::SetAsCurrent() const
{
    current = *this;
}


const Progress &Progress::GetCurrent()
{
    return current;
}


Progress Progress::GetCurrentStage()
{
    return Progress( current.stage );
}


const map<Progress::Stage, string> Progress::stage_formats =
 { { Progress::BUILDING_STEPS, "B" },
   { Progress::PLANNING_ONE,   "P" },
   { Progress::PLANNING_TWO,   "Q" },
   { Progress::PLANNING_THREE, "R" },
   { Progress::PARSING,        "I" }, 
   { Progress::TRANSFORMING,   "T" },
   { Progress::RENDERING,      "O" } };

Progress Progress::current;
