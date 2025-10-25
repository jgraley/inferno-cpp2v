#include "identifier_by_name_agent.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "vn/agents/agent.hpp"
#include "vn/agents/standard_agent.hpp"
#include "vn/scr_engine.hpp"
#include "vn/search_replace.hpp"
#include "vn/link.hpp"
#include "vn/lang/render.hpp"
#include "vn/sym/result.hpp"
#include "vn/sym/lazy_eval.hpp"
#include "vn/sym/boolean_operators.hpp"
#include "vn/sym/symbol_operators.hpp"
#include "vn/sym/set_operators.hpp"

using namespace CPPTree;

//---------------------------------- IdentifierByNameAgent ------------------------------------    

SYM::Lazy<SYM::BooleanExpression> IdentifierByNameAgent::SymbolicNormalLinkedQueryPRed(PatternLink keyer_plink) const
{
    auto keyer_expr = SYM::MakeLazy<SYM::SymbolVariable>(keyer_plink);   
    return SYM::MakeLazy<IsIdentifierNamedOperator>(this, name, keyer_expr);
}


IdentifierByNameAgent::IsIdentifierNamedOperator::IsIdentifierNamedOperator( const IdentifierByNameAgent *iba_,
                                                                             string name_,
                                                                             shared_ptr<SYM::SymbolExpression> a_ ) :
    iba( iba_ ),
    a( a_ ),
    name( name_ )
{    
}                                                


shared_ptr<SYM::PredicateOperator> IdentifierByNameAgent::IsIdentifierNamedOperator::Clone() const
{
    return make_shared<IsIdentifierNamedOperator>( iba, name, a );
}


list<shared_ptr<SYM::SymbolExpression> *> IdentifierByNameAgent::IsIdentifierNamedOperator::GetSymbolOperandPointers()
{
    return { &a };
}


unique_ptr<SYM::BooleanResult> IdentifierByNameAgent::IsIdentifierNamedOperator::Evaluate( const EvalKit &kit,
                                                                                           list<unique_ptr<SYM::SymbolicResult>> &&op_results ) const 
{
	(void)kit;
    ASSERT( op_results.size()==1 );        
    unique_ptr<SYM::SymbolicResult> ra = SoloElementOf(move(op_results));
    if( !ra->IsDefinedAndUnique() )
        return make_unique<SYM::BooleanResult>( false );
    
    TreePtr<Node> base_x = ra->GetOnlyXLink().GetChildTreePtr(); // TODO dynamic_pointer_cast support for TreePtrInterface #27
    if( auto si_x = DynamicTreePtrCast<CPPTree::SpecificIdentifier>(base_x) )
    {
        TRACE("Comparing ")(si_x->GetRenderTerminal())(" with ")(name);
        if( si_x->GetRenderTerminal() == name )
        {
            TRACE(" : same\n");
            return make_unique<SYM::BooleanResult>( true );
        }
        TRACE(" : different\n");
    }
    return make_unique<SYM::BooleanResult>( false );
}


shared_ptr<SYM::SymbolExpression> IdentifierByNameAgent::IsIdentifierNamedOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SYM::SymbolVariable> target ) const
{
    pair<TreePtr<Node>, TreePtr<Node>> range_nodes = iba->GetBounds( name );
    auto lower = make_shared<SYM::SymbolConstant>( range_nodes.first );
    auto upper = make_shared<SYM::SymbolConstant>( range_nodes.second );
    auto r = make_shared<SYM::AllInSimpleCompareRangeOperator>( lower, BoundingRole::NONE, true, upper, BoundingRole::NONE, true );
    return a->TrySolveForToEqual( kit, target, r );
}                                                                                                                                             
                                              
                                              
Orderable::Diff IdentifierByNameAgent::IsIdentifierNamedOperator::OrderCompare3WayCovariant( const Orderable &right, 
                                                                                             OrderProperty order_property ) const 
{
	(void)order_property;
    auto &r = GET_THAT_REFERENCE(right);
    return name.compare(r.name);
}  


string IdentifierByNameAgent::IsIdentifierNamedOperator::RenderNF() const
{
    return a->Render() + " ≅ '" + name + "'"; 
}


SYM::Expression::Precedence IdentifierByNameAgent::IsIdentifierNamedOperator::GetPrecedenceNF() const
{
    return Precedence::COMPARE;
}


Syntax::Production IdentifierByNameAgent::GetAgentProduction() const
{
	return Syntax::Production::PRIMITIVE_EXPR;
}


string IdentifierByNameAgent::GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const
{
    (void)kit;
	(void)surround_prod;
	string s = "⊜【" + GetIdentifierSubTypeName();
	s += ",'" + name + "'";
	s += "】";
	return s;
} 
  
    
string IdentifierByNameAgent::GetCouplingNameHint() const
{
	string t = GetIdentifierSubTypeName();
	transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return tolower(c); });
	return "found_" + t + "_id";
} 


Graphable::NodeBlock IdentifierByNameAgent::GetGraphBlockInfo() const
{
    // The IdentifierByNameBase node appears as a trapezium (rectangle narrower at the top) with
    // the string that must be matched inside it.
    // TODO indicate whether it's matching instance, label or type identifier
    // Update: PreRestriction indicator seems to be doing that now
    NodeBlock block;
    block.bold = true;
    block.title = "⊜'" + name + "'";    
    block.shape = "trapezium";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    return block;
}

//---------------------------------- InstanceIdentifierByNameAgent ------------------------------------    

pair<TreePtr<Node>, TreePtr<Node>> InstanceIdentifierByNameAgent::GetBounds( string name ) const
{
    TreePtr<Node> minimus = MakeTreeNode<SpecificInstanceIdentifier>( name, Orderable::BoundingRole::MINIMUS );
    TreePtr<Node> maximus = MakeTreeNode<SpecificInstanceIdentifier>( name, Orderable::BoundingRole::MAXIMUS );
    return make_pair( minimus, maximus );
}

string InstanceIdentifierByNameAgent::GetIdentifierSubTypeName() const
{
	return "Instance";
}  

//---------------------------------- TypeIdentifierByNameAgent ------------------------------------    

pair<TreePtr<Node>, TreePtr<Node>> TypeIdentifierByNameAgent::GetBounds( string name ) const
{
    TreePtr<Node> minimus = MakeTreeNode<SpecificTypeIdentifier>( name, Orderable::BoundingRole::MINIMUS );
    TreePtr<Node> maximus = MakeTreeNode<SpecificTypeIdentifier>( name, Orderable::BoundingRole::MAXIMUS );
    return make_pair( minimus, maximus );
}


string TypeIdentifierByNameAgent::GetIdentifierSubTypeName() const
{
	return "Type";
}  

//---------------------------------- LabelIdentifierByNameAgent ------------------------------------    

pair<TreePtr<Node>, TreePtr<Node>> LabelIdentifierByNameAgent::GetBounds( string name ) const
{
    TreePtr<Node> minimus = MakeTreeNode<SpecificLabelIdentifier>( name, Orderable::BoundingRole::MINIMUS );
    TreePtr<Node> maximus = MakeTreeNode<SpecificLabelIdentifier>( name, Orderable::BoundingRole::MAXIMUS );
    return make_pair( minimus, maximus );
}


string LabelIdentifierByNameAgent::GetIdentifierSubTypeName() const
{
	return "Label";
}  

//---------------------------------- PreprocessorIdentifierByNameAgent ------------------------------------    

pair<TreePtr<Node>, TreePtr<Node>> PreprocessorIdentifierByNameAgent::GetBounds( string name ) const
{
    TreePtr<Node> minimus = MakeTreeNode<SpecificPreprocessorIdentifier>( name, Orderable::BoundingRole::MINIMUS );
    TreePtr<Node> maximus = MakeTreeNode<SpecificPreprocessorIdentifier>( name, Orderable::BoundingRole::MAXIMUS );
    return make_pair( minimus, maximus );
}


string PreprocessorIdentifierByNameAgent::GetIdentifierSubTypeName() const
{
	return "Preprocessor";
}  
