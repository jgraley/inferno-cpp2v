#ifndef EXPRESSION_ANALYSIS_HPP
#define EXPRESSION_ANALYSIS_HPP

#include "common/common.hpp"
#include "common/orderable.hpp"

#include "expression.hpp"

#include <functional>

using namespace std;

namespace SYM
{

// -------------------------- ExpressionWalker ----------------------------    
    
class ExpressionWalker
{
public:    
    typedef function<bool(shared_ptr<Expression>)> Lambda;
    ExpressionWalker( const Lambda &lambda );

    void operator()( shared_ptr<Expression> expr );
    
private:
    const Lambda lambda;
};

// -------------------------- PredicateAnalysis ----------------------------    

class PredicateAnalysis
{
public:    
    static void CheckRegularPredicateForm( shared_ptr<Expression> expr );
    static list<shared_ptr<BooleanExpression>> GetPredicates( shared_ptr<Expression> expr );
};
   
};

#endif
