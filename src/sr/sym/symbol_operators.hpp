#ifndef SYMBOL_OPERATORS_HPP
#define SYMBOL_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "../db/x_tree_database.hpp"

namespace SYM
{ 

// ------------------------- SymbolConstant --------------------------

class SymbolConstant : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit SymbolConstant( SR::XLink xlink );
    explicit SymbolConstant( TreePtr<Node> node );
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const override;    
    unique_ptr<SymbolResultInterface> GetValue() const;
    SR::XLink GetOnlyXLink() const;

    Orderable::Diff OrderCompare3WayLocal( const Orderable &right, 
                                           OrderProperty order_property ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const SR::XLink xlink;
};

// ------------------------- SymbolVariable --------------------------

class SymbolVariable : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit SymbolVariable( const SR::PatternLink &plink );
    virtual set<SR::PatternLink> GetRequiredVariables() const override;
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const override;
    SR::PatternLink GetPatternLink() const;

    virtual shared_ptr<SymbolExpression> TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                       shared_ptr<SymbolExpression> to_equal ) const override;

    Orderable::Diff OrderCompare3WayLocal( const Orderable &right, 
                                           OrderProperty order_property ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;

private:
    const SR::PatternLink plink;
};

// ------------------------- ChildToSymbolOperator --------------------------

class ChildToSymbolOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit ChildToSymbolOperator( TreePtr<Node> archetype_node,
                                      int item, 
                                      shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                               list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const = 0;

    Orderable::Diff OrderCompare3WayLocal( const Orderable &right, 
                                       OrderProperty order_property ) const override;                                                

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    virtual string GetItemTypeName() const = 0;
    
protected:
    const TreePtr<Node> archetype_node;
    const int item_index;
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- ChildOperator --------------------------

class ChildOperator : public ChildToSymbolOperator
{
    using ChildToSymbolOperator::ChildToSymbolOperator;
    virtual shared_ptr<SymbolExpression> TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const;
};

// ------------------------- ChildSequenceFrontOperator --------------------------

class ChildSequenceFrontOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildSequenceBackOperator --------------------------

class ChildSequenceBackOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildCollectionFrontOperator --------------------------

class ChildCollectionFrontOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- SingularChildOperator --------------------------

class SingularChildOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- XTreeDbToSymbolOperator --------------------------

class XTreeDbToSymbolOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit XTreeDbToSymbolOperator( shared_ptr<SymbolExpression> a );
    VariablesRequiringRows GetVariablesRequiringRows() const override;
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                               list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;
    virtual SR::XLink EvalXLinkFromRow( SR::XLink parent_xlink, 
                                           const SR::LinkTable::Row &row ) const = 0;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    virtual string GetRenderPrefix() const = 0;
    
protected:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- ParentOperator --------------------------

class ParentOperator : public XTreeDbToSymbolOperator
{
public:    
    using XTreeDbToSymbolOperator::XTreeDbToSymbolOperator;
    virtual SR::XLink EvalXLinkFromRow( SR::XLink parent_xlink, 
                                           const SR::LinkTable::Row &row ) const override;

    virtual shared_ptr<SymbolExpression> TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const override;

    virtual string GetRenderPrefix() const override;
};

// ------------------------- LastDescendantOperator --------------------------

class LastDescendantOperator : public XTreeDbToSymbolOperator
{
public:    
    using XTreeDbToSymbolOperator::XTreeDbToSymbolOperator;
    virtual SR::XLink EvalXLinkFromRow( SR::XLink parent_xlink, 
                                           const SR::LinkTable::Row &row ) const override;
    virtual string GetRenderPrefix() const override;
};

// ------------------------- MyContainerFrontOperator --------------------------

class MyContainerFrontOperator : public XTreeDbToSymbolOperator
{
public:    
    using XTreeDbToSymbolOperator::XTreeDbToSymbolOperator;
    virtual SR::XLink EvalXLinkFromRow( SR::XLink parent_xlink, 
                                           const SR::LinkTable::Row &row ) const override;
    virtual string GetRenderPrefix() const override;
};

// ------------------------- MyContainerBackOperator --------------------------

class MyContainerBackOperator : public XTreeDbToSymbolOperator
{
public:    
    using XTreeDbToSymbolOperator::XTreeDbToSymbolOperator;
    virtual SR::XLink EvalXLinkFromRow( SR::XLink parent_xlink, 
                                           const SR::LinkTable::Row &row ) const override;
    virtual string GetRenderPrefix() const override;
};

// ------------------------- MySequenceSuccessorOperator --------------------------

class MySequenceSuccessorOperator : public XTreeDbToSymbolOperator
{
public:    
    using XTreeDbToSymbolOperator::XTreeDbToSymbolOperator;
    virtual SR::XLink EvalXLinkFromRow( SR::XLink parent_xlink, 
                                           const SR::LinkTable::Row &row ) const override;

    virtual shared_ptr<SymbolExpression> TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const override;

    virtual string GetRenderPrefix() const override;
};

// ------------------------- MySequencePredecessorOperator --------------------------

class MySequencePredecessorOperator : public XTreeDbToSymbolOperator
{
public:    
    using XTreeDbToSymbolOperator::XTreeDbToSymbolOperator;
    virtual SR::XLink EvalXLinkFromRow( SR::XLink parent_xlink, 
                                           const SR::LinkTable::Row &row ) const override;

    virtual shared_ptr<SymbolExpression> TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const override;

    virtual string GetRenderPrefix() const override;
};

// ------------------------- AllChildrenOperator --------------------------

class AllChildrenOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit AllChildrenOperator( shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                               list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;

    virtual shared_ptr<SymbolExpression> TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const override;
                                             
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
protected:
    const shared_ptr<SymbolExpression> a;
};
};

#endif // include guard
