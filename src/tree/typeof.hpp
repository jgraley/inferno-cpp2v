#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "helpers/transformation.hpp"
#include "cpptree.hpp"

// Really a GetTypeOf class, but named for the way it appears in pattern graphs
class TypeOf : public Transformation
{
public:
    class UnsupportedMismatch : public Mismatch {};
    class UnsupportedExpressionMismatch : public UnsupportedMismatch {};
    class UnsupportedOperatorMismatch : public UnsupportedMismatch {};
    class UnsupportedThisMismatch : public UnsupportedMismatch {}; // TODO add support
    class UnsupportedSpecialMismatch : public UnsupportedMismatch {};
    class UnsupportedLiteralMismatch : public UnsupportedMismatch {};
    class DeleteUnsupportedMismatch : public UnsupportedMismatch {};
    class UsageMismatch : public Mismatch {};
    class NumericalOperatorUsageMismatch : public UsageMismatch {};
    class NumericalOperatorUsageMismatch1 : public NumericalOperatorUsageMismatch {};
    class NumericalOperatorUsageMismatch2 : public NumericalOperatorUsageMismatch {};
    class NumericalOperatorUsageMismatch3 : public NumericalOperatorUsageMismatch {};
    class NumericalOperatorUsageMismatch4 : public NumericalOperatorUsageMismatch {};
    class DereferenceUsageMismatch : public UsageMismatch {};

    // If you don't have a kit, use Transformation::operator().
    // If you do but you only have a Node, use this
    AugTreePtr<Node> TryApplyTransformation( const TransKit &kit, AugTreePtr<Node> node ) const override;
    
    // If you have kit and an Expression, well done! You can call this one
    AugTreePtr<CPPTree::Type> Get( const TransKit &kit, AugTreePtr<CPPTree::Expression> o ) const;

    // Is this call really a constructor call? If so return the object being
    // constructed. Otherwise, return nullptr
    AugTreePtr<CPPTree::Expression> TryGetConstructedExpression( const TransKit &kit, AugTreePtr<CPPTree::GoSub> gs ) const;

private:    
    AugTreePtr<CPPTree::Type> GetOperator( const TransKit &kit, AugTreePtr<CPPTree::Operator> op, list<AugTreePtr<CPPTree::Type>> optypes ) const;
    AugTreePtr<CPPTree::Type> GetStandard( const TransKit &kit, list<AugTreePtr<CPPTree::Type>> &optypes ) const;
    AugTreePtr<CPPTree::Type> GetStandardOnNumerics( const TransKit &kit, list<AugTreePtr<CPPTree::Numeric>> &optypes ) const;
    AugTreePtr<CPPTree::Type> GetSpecial( const TransKit &kit, AugTreePtr<CPPTree::Operator> op, list<AugTreePtr<CPPTree::Type>> &optypes ) const;
    AugTreePtr<CPPTree::Type> GetLiteral( const TransKit &kit, AugTreePtr<CPPTree::Literal> l ) const;
    
public:
    static TypeOf instance; 
};

#endif

