#include "pattern_transformation.hpp"
#include "vn_step.hpp"
#include "agents/slave_agent.hpp"

using namespace SR;
   
void PatternTransformationVector::operator()( VNStep &vnt ) 
{
    for( shared_ptr<PatternTransformation> t : *this )
    {
        // TODO give the name of it, by making PatternTransformation traceable
        TRACE("Running pattern transformation\n"); 
        (*t)(vnt);
    }                            
}		                           


