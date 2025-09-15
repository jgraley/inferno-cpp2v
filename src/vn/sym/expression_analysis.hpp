#ifndef EXPRESSION_ANALYSIS_HPP
#define EXPRESSION_ANALYSIS_HPP

#include "common/common.hpp"
#include "common/orderable.hpp"

#include "expression.hpp"

#include <functional>

using namespace std;

namespace SYM
{
class PredicateOperator;
    
// -------------------------- ExpressionWalker ----------------------------    
    
class ExpressionWalker
{
public:    
    typedef function<bool(shared_ptr<Expression>)> Lambda;
    ExpressionWalker( bool include_root, const Lambda &lambda );

    void operator()( shared_ptr<Expression> expr, bool is_root = true );
    
private:
    const bool include_root;
    const Lambda lambda;
};

// -------------------------- PredicateAnalysis ----------------------------    

class PredicateAnalysis
{
public:    
    static void CheckRegularPredicateForm( shared_ptr<Expression> expr );
    static void CheckNoPredicatesUnder( shared_ptr<Expression> expr );
    static vector< set<shared_ptr<PredicateOperator>> > GetPredicates( shared_ptr<Expression> expr );
};
   
};

#endif
