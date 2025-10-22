#include "inferno_agents.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "vn/agents/agent.hpp"
#include "vn/agents/standard_agent.hpp"
#include "vn/scr_engine.hpp"
#include "vn/search_replace.hpp"
#include "vn/link.hpp"
#include "vn/sym/result.hpp"
#include "vn/sym/lazy_eval.hpp"
#include "vn/sym/boolean_operators.hpp"
#include "vn/sym/symbol_operators.hpp"
#include "vn/sym/set_operators.hpp"
#include "vn/lang/render.hpp"

// Not pulling in SYM because it clashes with CPPTree
//using namespace SYM;
using namespace CPPTree;

//---------------------------------- BuildIdentifierAgent ------------------------------------    

TreePtr<Node> BuildIdentifierAgent::BuildNewSubtree(const SCREngine *acting_engine)
{
    return BuildSpecificIdentifier( GetNewName(acting_engine) ); 
}


string BuildIdentifierAgent::GetNewName(const SCREngine *acting_engine)
{
    INDENT("B");
    TRACE("Begin SoftMakeIdentifier recurse for \"")(format)("\"\n");
    vector<string> vs;
    bool all_same = true;
    for( TreePtrInterface &source : sources )
    {
        ASSERT( source );
        // We have a child identifier - let replace algorithm run in the expectation it will
        // get subsitituted with a SpecificIdentifier from the original program tree
        PatternLink source_plink(&source);
        TreePtr<Node> new_identifier = source_plink.GetChildAgent()->BuildForBuildersAnalysis(source_plink, acting_engine);
        TRACE("End SoftMakeIdentifier recurse\n");
        ASSERT( new_identifier );
        TreePtr<SpecificIdentifier> si = DynamicTreePtrCast<SpecificIdentifier>( new_identifier );
        ASSERT( si )("BuildIdentifier: ")(*new_identifier)(" should be a kind of SpecificIdentifier (format is %s)", format.c_str());
        string s = si->GetRenderTerminal();
        if( !vs.empty() )
            all_same = all_same && (s == vs.back());
        vs.push_back( s );
    }

    // Optional functionality: when every identifier has the same name, just return that
    // name. Handy for "merging" operations.
    if( (flags & BYPASS_WHEN_IDENTICAL) && all_same )
        return vs[0];  

    // Use sprintf to build a new identifier based on the found one. Obviously %s
    // becomes the old identifier's name.
    switch( vs.size() )
    {
        case 0:
            return SSPrintf( format.c_str() );
        case 1:
            return SSPrintf( format.c_str(), vs[0].c_str() );
        case 2:
            return SSPrintf( format.c_str(), vs[0].c_str(), vs[1].c_str() );
        default:
            ASSERTFAIL("Please add more cases to GetNewName()");
    }
}

Syntax::Production BuildIdentifierAgent::GetAgentProduction() const
{
	return Syntax::Production::VN_PREFIX;
}


string BuildIdentifierAgent::GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const
{
	(void)surround_prod;
	// Note: we are not differentiating between different kinds of identifier
	// (type, instance, label, preproc (which doens't have support here anyway)).
	// TODO drop these subclasses. TODO:
	// - Loosen up the pointer types
	// - This class derives from Special<CPPTree::SpecificIdentifier> (if we're still doing that)
	// - Move BuildSpecificIdentifier() into nodes like Type, Expression etc
	// - Get parent pointer archetype and call on that.
	// OR JUST SHOVE IN WHATEVER FITS
	string s = (flags & BYPASS_WHEN_IDENTICAL) ? "⧇" : "⧈";
	s += "【'" + format + "'】";
	list<string> ls;
	Sequence<CPPTree::Identifier> scopy = sources;
	for( TreePtrInterface &source : scopy )
		ls.push_back( kit.render( (TreePtr<Node>)source, Syntax::Production::VN_SEP ) );
	s += Join( ls, "┆ ", "⦑ ", " ⦒");
	return s;
} 
  
    
string BuildIdentifierAgent::GetCouplingNameHint() const
{
	return "new_id"; 
} 


Graphable::NodeBlock BuildIdentifierAgent::GetGraphBlockInfo() const
{
    // The BuildIdentifier node appears as a parallelogram (rectangle pushed to the side) with
    // the printf format string that controls the name of the generated identifier inside it.
    // TODO indicate whether it's building instance, label or type identifier
    NodeBlock block;
    block.bold = true;
    block.title = "⧇'"+format+"'"; // text from program code, so use single quotes
    block.shape = "parallelogram";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.item_blocks = Node::GetSubblocks(const_cast<Sequence<CPPTree::Identifier> *>(&sources), phase);
    return block;
}

//---------------------------------- BuildInstanceIdentifierAgent ------------------------------------    

TreePtr<CPPTree::SpecificIdentifier> BuildInstanceIdentifierAgent::BuildSpecificIdentifier(string name) const
{
    return MakeTreeNode<CPPTree::SpecificInstanceIdentifier>( name ); 
}

//---------------------------------- BuildTypeIdentifierAgent ------------------------------------    

TreePtr<CPPTree::SpecificIdentifier> BuildTypeIdentifierAgent::BuildSpecificIdentifier(string name) const
{
    return MakeTreeNode<CPPTree::SpecificTypeIdentifier>( name ); 
}

//---------------------------------- BuildLabelIdentifierAgent ------------------------------------    

TreePtr<CPPTree::SpecificIdentifier> BuildLabelIdentifierAgent::BuildSpecificIdentifier(string name) const
{
    return MakeTreeNode<CPPTree::SpecificLabelIdentifier>( name ); 
}

//---------------------------------- StringizeAgent ------------------------------------    

TreePtr<Node> StringizeAgent::BuildNewSubtree(const SCREngine *acting_engine)
{
    PatternLink source_plink(&source);
    TreePtr<Node> new_identifier = source_plink.GetChildAgent()->BuildForBuildersAnalysis(source_plink, acting_engine);
    return MakeTreeNode<CPPTree::SpecificString>( new_identifier->GetRenderTerminal() ); 
}


Syntax::Production StringizeAgent::GetAgentProduction() const
{
	return Syntax::Production::VN_PREFIX;
}


string StringizeAgent::GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const
{
	(void)surround_prod;
	return "§" + kit.render( source, Syntax::Production::VN_PREFIX );
} 

    
string StringizeAgent::GetCouplingNameHint() const
{
	return "stringize"; 
} 


Graphable::NodeBlock StringizeAgent::GetGraphBlockInfo() const
{
    NodeBlock block;
    block.bold = true;
    block.title = "Stringize"; 
    block.symbol = "§";
    block.shape = "parallelogram";    
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.item_blocks = Node::GetSubblocks(const_cast<TreePtr<CPPTree::Identifier> *>(&source), phase);
    return block;
}
	
//---------------------------------- IdentifierByNameAgent ------------------------------------    

Graphable::NodeBlock IdentifierByNameAgent::GetGraphBlockInfo() const
{
    // The IdentifierByNameBase node appears as a trapezium (rectangle narrower at the top) with
    // the string that must be matched inside it.
    // TODO indicate whether it's matching instance, label or type identifier
    // Update: PreRestriction indicator seems to be doing that now
    NodeBlock block;
    block.bold = true;
    block.title = "'" + name + "'?";    
    block.shape = "trapezium";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    return block;
}


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

//---------------------------------- InstanceIdentifierByNameAgent ------------------------------------    

pair<TreePtr<Node>, TreePtr<Node>> InstanceIdentifierByNameAgent::GetBounds( string name ) const
{
    TreePtr<Node> minimus = MakeTreeNode<SpecificInstanceIdentifier>( name, Orderable::BoundingRole::MINIMUS );
    TreePtr<Node> maximus = MakeTreeNode<SpecificInstanceIdentifier>( name, Orderable::BoundingRole::MAXIMUS );
    return make_pair( minimus, maximus );
}

//---------------------------------- TypeIdentifierByNameAgent ------------------------------------    

pair<TreePtr<Node>, TreePtr<Node>> TypeIdentifierByNameAgent::GetBounds( string name ) const
{
    TreePtr<Node> minimus = MakeTreeNode<SpecificTypeIdentifier>( name, Orderable::BoundingRole::MINIMUS );
    TreePtr<Node> maximus = MakeTreeNode<SpecificTypeIdentifier>( name, Orderable::BoundingRole::MAXIMUS );
    return make_pair( minimus, maximus );
}

//---------------------------------- LabelIdentifierByNameAgent ------------------------------------    

pair<TreePtr<Node>, TreePtr<Node>> LabelIdentifierByNameAgent::GetBounds( string name ) const
{
    TreePtr<Node> minimus = MakeTreeNode<SpecificLabelIdentifier>( name, Orderable::BoundingRole::MINIMUS );
    TreePtr<Node> maximus = MakeTreeNode<SpecificLabelIdentifier>( name, Orderable::BoundingRole::MAXIMUS );
    return make_pair( minimus, maximus );
}

//---------------------------------- PreprocessorIdentifierByNameAgent ------------------------------------    

pair<TreePtr<Node>, TreePtr<Node>> PreprocessorIdentifierByNameAgent::GetBounds( string name ) const
{
    TreePtr<Node> minimus = MakeTreeNode<SpecificPreprocessorIdentifier>( name, Orderable::BoundingRole::MINIMUS );
    TreePtr<Node> maximus = MakeTreeNode<SpecificPreprocessorIdentifier>( name, Orderable::BoundingRole::MAXIMUS );
    return make_pair( minimus, maximus );
}

//---------------------------------- BuildContainerSizeAgent ------------------------------------    

TreePtr<Node> BuildContainerSizeAgent::BuildNewSubtree(const SCREngine *acting_engine)
{
    ASSERT( container );
    PatternLink container_plink(&container);
    TreePtr<Node> new_node = container_plink.GetChildAgent()->BuildForBuildersAnalysis(container_plink, acting_engine);
    ASSERT( new_node );
    ContainerInterface *new_container = dynamic_cast<ContainerInterface *>(new_node.get());
    ASSERT( new_container );
    int size = new_container->size();
    return MakeTreeNode<SpecificInteger>(size); 
}                                                   


Graphable::NodeBlock BuildContainerSizeAgent::GetGraphBlockInfo() const
{

    NodeBlock block;
    block.bold = false;
    block.title = GetName();
    block.shape = "egg";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    if( container )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(container.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &container );
        block.item_blocks.push_back( { "container", 
                                      "", 
                                      false,
                                      { link } } );
    }
    return block;
}

