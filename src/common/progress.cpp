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
    for( auto p : Progress::stage_info )
    {
        stage = p.first;
        string stage_code = p.second.code;
        if( s.substr(0, stage_code.size()) != stage_code)
            continue;
            
        switch( p.second.steppiness )
        {
        case NON_STEPPY:
            if( s.size() != stage_code.size() )
                break;            
            step = NO_STEP;
            return; // success: no step        
        
        case STEPPY:        
            string ss = s.substr(stage_code.size());                
            step = atoi(ss.c_str());
            if( step==0 && ss != "0" )
                break; // error - invalid            
            return; // done - got a step
        }
    }
    
    // Didn't find stage
    stage = INVALID;
    step = -1;
}


string Progress::GetPrefix(int width) const
{
    if( stage==INVALID )
        return string( width, ' '); // defensive
        
    string stage_code = stage_info.at(stage).code;
    int width_after_stage_code = max((string::size_type)0, width-stage_code.size());
    
    switch( stage_info.at(stage).steppiness )
    {
    case NON_STEPPY:
        if( width==0 )
            return stage_code;
        else
            return stage_code + string( width_after_stage_code, ' ');
    
    case STEPPY:        
         if( width==0 )
             return stage_code + to_string(step);   
         else
             return stage_code + SSPrintf( "%0"+to_string(width_after_stage_code)+"d", step );
    }
    return ""; // Can't ASSERT here    
}


int Progress::GetStep() const
{
    return step;
}


Progress::Stage Progress::GetStage() const
{
    return stage;
}


Progress::Steppiness Progress::GetSteppiness() const
{
    return stage_info.at(stage).steppiness;
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


const map<Progress::Stage, Progress::StageInfoBlock> Progress::stage_info =
 { { Progress::BUILDING_STEPS, {"B", NON_STEPPY} }, // Building could be steppy #385
   { Progress::PATTERN_TRANS,  {"X", STEPPY} },  
   { Progress::PLANNING_ONE,   {"P", STEPPY} },
   { Progress::PLANNING_TWO,   {"Q", STEPPY} },
   { Progress::PLANNING_THREE, {"R", STEPPY} },
   { Progress::PARSING,        {"I", NON_STEPPY} }, 
   { Progress::TRANSFORMING,   {"T", STEPPY} },
   { Progress::RENDERING,      {"O", NON_STEPPY} } };

Progress Progress::current;
