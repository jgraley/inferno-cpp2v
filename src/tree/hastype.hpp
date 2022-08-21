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

    virtual TreePtr<Node> operator()( const TreeKit &kit_, TreePtr<Node> node );

    // Is this call really a constructor call? If so return the object being
    // constructed. Otherwise, return nullptr
    TreePtr<CPPTree::Expression> IsConstructorCall( const TreeKit &kit_, TreePtr<CPPTree::Call> call );

private:    
    // TODO make these private
    TreePtr<CPPTree::Type> Get( TreePtr<CPPTree::Expression> o );
    TreePtr<CPPTree::Type> Get( TreePtr<CPPTree::Operator> op, Sequence<CPPTree::Type> optypes );
    TreePtr<CPPTree::Type> GetStandard( Sequence<CPPTree::Type> &optypes );
    TreePtr<CPPTree::Type> GetStandard( Sequence<CPPTree::Numeric> &optypes );
    TreePtr<CPPTree::Type> GetSpecial( TreePtr<CPPTree::Operator> op, Sequence<CPPTree::Type> &optypes );
    TreePtr<CPPTree::Type> GetLiteral( TreePtr<CPPTree::Literal> l );

	const TreeKit *kit = nullptr;
    
public:
    static HasType instance; 
};

#endif

