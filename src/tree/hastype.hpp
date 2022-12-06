#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "helpers/transformation.hpp"
#include "cpptree.hpp"

// Really a GetTypeOf class, but named for the way it appears in pattern graphs
class HasType : public Transformation
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

    AugTreePtr<Node> ApplyTransformation( const TreeKit &kit_, TreePtr<Node> node ) const override;

    // Is this call really a constructor call? If so return the object being
    // constructed. Otherwise, return nullptr
    AugTreePtr<CPPTree::Expression> IsConstructorCall( const TreeKit &kit_, TreePtr<CPPTree::Call> call ) const;

private:    
    // TODO make these private
    AugTreePtr<CPPTree::Type> Get( TreePtr<CPPTree::Expression> o ) const;
    AugTreePtr<CPPTree::Type> Get( TreePtr<CPPTree::Operator> op, list<AugTreePtr<CPPTree::Type>> optypes ) const;
    AugTreePtr<CPPTree::Type> GetStandard( list<AugTreePtr<CPPTree::Type>> &optypes ) const;
    AugTreePtr<CPPTree::Type> GetStandard( list<AugTreePtr<CPPTree::Numeric>> &optypes ) const;
    AugTreePtr<CPPTree::Type> GetSpecial( TreePtr<CPPTree::Operator> op, list<AugTreePtr<CPPTree::Type>> &optypes ) const;
    AugTreePtr<CPPTree::Type> GetLiteral( TreePtr<CPPTree::Literal> l ) const;

	const mutable TreeKit *kit = nullptr;
    
public:
    static HasType instance; 
};

#endif

