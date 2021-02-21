#include "progress.hpp"

#include "standard.hpp"

#include <algorithm>

Progress::Progress( Stage stage_, int step_ ) :
    stage(stage_),
    step(step_)
{    
}


string Progress::GetPrefix(int width) const
{
    string stage_code;
    bool step_is_valid;
    tie( stage_code, step_is_valid ) = stage_formats.at(stage);
    int width_after_stage_code = max((string::size_type)0, width-stage_code.size());
    
    if( width==0 )
        return stage_code + (step_is_valid ? to_string(step) : string());        
    else if( step_is_valid )
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


void Progress::SetAsCurrent() const
{
    current = *this;
}


const Progress &Progress::GetCurrent()
{
    return current;
}


const map< Progress::Stage, pair<string, bool> > Progress::stage_formats =
 { { Progress::BUILDING_STEPS, make_pair("B", false) },
   { Progress::PLANNING_ONE,   make_pair("P", true) },
   { Progress::PLANNING_TWO,   make_pair("Q", true) },
   { Progress::PARSING,        make_pair("I", false) }, 
   { Progress::TRANSFORMING,   make_pair("T", true) },
   { Progress::RENDERING,      make_pair("O", false) } };

Progress Progress::current;
