#include "pattern_transformation.hpp"
#include "vn_transformation.hpp"
#include "agents/slave_agent.hpp"

using namespace SR;
   
void PatternTransformationVector::operator()( VNTransformation &vnt ) 
{
    FOREACH( shared_ptr<PatternTransformation> t, *this )
    {
        // TODO give the name of it, by making PatternTransformation traceable
        TRACE("Running pattern transformation\n"); 
        (*t)(vnt);
    }                            
}		                           


