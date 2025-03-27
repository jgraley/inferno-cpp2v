#ifndef INVENTED_HPP
#define INVENTED_HPP

#include "common/common.hpp"
#include "common/progress.hpp"

/* Not terribly clean right now. Of course this could become a 
 * fully-fledged attribute system but therer would still be a question 
 * on how to do the "invented" attribute. We could expand fully into 
 * Progress, but doesn't feel quite right. Also not too keen on using
 * shared_ptr or ValuePtr due to processing cost. Maybe better 
 * to store an Agent * (we can forward declare it and agents live
 * long enough for C pointer to be fine). Can then get to step via eg
 * agent->GetSCREngine()->GetSearchReplace()->GetVNStep()->GetStepIndex().
 * Agent does SetInvented(this);
 */ 

/// Track the "invention" of nodes. Invention propagates through 
/// clone and duplicate but does not appear in an itemisation.
/// It provides a "generation number" mechanism. No _FUNCTION
/// method is required.
class Invented
{
public: 
    Invented() :
        step(Progress::NO_STEP)
    {
    }
    
    void SetInventedHere()
    {
        step = Progress::GetCurrent().GetStep();
        ASSERT( step != Progress::NO_STEP );                
    }
    
    bool WasInventedDuringCurrentStep() const
    {
        return (Progress::GetCurrent().GetStep() == step);
    }

    int step;
};

#endif
