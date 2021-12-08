#ifndef REWRITES_HPP
#define REWRITES_HPP

#include "lazy.hpp"

namespace SR
{ 
    class Agent;
};

namespace SYM
{     
    typedef SR::Agent *Metadata;
    
    typedef list< pair<SYM::Lazy<SYM::BooleanExpression>, Metadata> > BooleanExpressionList;
    
    class Splitter
    {
    public:
        BooleanExpressionList operator()( BooleanExpressionList bel_in ) const;
    };
};

#endif
