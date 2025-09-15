#include "pattern_transformation.hpp"
#include "vn_step.hpp"
#include "agents/embedded_scr_agent.hpp"

using namespace VN;

bool PatternTransformation::RequireAgentsToIncomingPlinksMap() const
{
	return false; // no thanks
	// PLink memory safety: if you say "yes" to this, you are responsible 
	// for keeping the plinks in agents_to_incoming_plinks valid or removing them from the map.
}


   
void PatternTransformationVector::operator()( VNStep &vnt ) 
{
    for( shared_ptr<PatternTransformation> t : *this )
    {
        // TODO give the name of it, by making PatternTransformation traceable
        TRACE("Running pattern transformation\n"); 
        (*t)(vnt);
    }                            
}                                   
