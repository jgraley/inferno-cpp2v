#ifndef BOOLEAN_OPERATORS_HPP
#define BOOLEAN_OPERATORS_HPP

#include "operator.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

#include <functional>

namespace SYM
{ 

class BooleanLambda : public BooleanOperator
{
public:
    typedef function<void( const EvalKit &kit )> LambdaType;
    
    BooleanLambda( set<SR::PatternLink> input_plinks_,
                    const LambdaType &lambda_,
                    string description );
    virtual set<SR::PatternLink> GetInputPatternLinks() const override;
    virtual void Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    
private:
    const set<SR::PatternLink> input_plinks;
    const LambdaType lambda;
    const string description;
};


class AndOperator : public BooleanOperator
{
public:    
    AndOperator( shared_ptr<BooleanOperator> a,
                 shared_ptr<BooleanOperator> b );
    virtual set<SR::PatternLink> GetInputPatternLinks() const override;
    virtual void Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    
private:
    shared_ptr<BooleanOperator> a;
    shared_ptr<BooleanOperator> b;
};

};

#endif // include guard
