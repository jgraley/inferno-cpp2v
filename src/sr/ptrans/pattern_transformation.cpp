#include "pattern_transformation.hpp"
#include "vn_transformation.hpp"
#include "agents/slave_agent.hpp"

using namespace SR;
   
void PatternTransformationVector::operator()( VNTransformation &vnt ) 
{
    FOREACH( shared_ptr<PatternTransformation> t, *this )
    {
        (*t)(vnt);
    }                            
}		                           


