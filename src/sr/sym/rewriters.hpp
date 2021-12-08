#ifndef REWRITES_HPP
#define REWRITES_HPP

#include "lazy.hpp"

namespace SR
{ 
    class Agent;
};

namespace SYM
{         
    typedef list< SYM::Lazy<SYM::BooleanExpression> > BooleanExpressionList;
    
    class Splitter
    {
    public:
        BooleanExpressionList operator()( BooleanExpressionList bel_in ) const;
    };
};

#endif
