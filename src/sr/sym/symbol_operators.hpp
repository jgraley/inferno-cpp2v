#ifndef SYMBOL_OPERATORS_HPP
#define SYMBOL_OPERATORS_HPP

#include "expression.hpp"
#include "lazy.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SR
{
    class Agent;
};

namespace SYM
{ 

// ------------------------- ChildOperator --------------------------

class ChildOperator : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    ChildOperator( const SR::Agent *ref_agent,
                   int item, 
                   shared_ptr<SymbolExpression> a );
    virtual set<shared_ptr<Expression>> GetOperands() const override;
    virtual unique_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const = 0;
    virtual string GetItemType() const = 0;
    
private:
    const SR::Agent *ref_agent;
    const int item_index;
    shared_ptr<SymbolExpression> a;
};

// ------------------------- SequenceFrontChildOperator --------------------------

class SequenceFrontChildOperator : public ChildOperator
{
public:    
    SequenceFrontChildOperator( const SR::Agent *ref_agent,
                                int item_index, 
                                shared_ptr<SymbolExpression> a );

    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemType() const override;    
};

// ------------------------- SequenceBackChildOperator --------------------------

class SequenceBackChildOperator : public ChildOperator
{
public:    
    SequenceBackChildOperator( const SR::Agent *ref_agent,
                               int item_index, 
                               shared_ptr<SymbolExpression> a );

    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemType() const override;    
};

// ------------------------- CollectionFrontChildOperator --------------------------

class CollectionFrontChildOperator : public ChildOperator
{
public:    
    CollectionFrontChildOperator( const SR::Agent *ref_agent,
                                  int item_index, 
                                  shared_ptr<SymbolExpression> a );

    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemType() const override;    
};

// ------------------------- SingularChildOperator --------------------------

class SingularChildOperator : public ChildOperator
{
public:    
    SingularChildOperator( const SR::Agent *ref_agent,
                           int item_index, 
                           shared_ptr<SymbolExpression> a );

    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemType() const override;    
};

};

#endif // include guard
