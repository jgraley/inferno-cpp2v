#include "stringize_agent.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "vn/agents/agent.hpp"
#include "vn/agents/standard_agent.hpp"
#include "vn/scr_engine.hpp"
#include "vn/search_replace.hpp"
#include "vn/link.hpp"
#include "vn/lang/render.hpp"

// Not pulling in SYM because it clashes with CPPTree
//using namespace SYM;
using namespace CPPTree;

//---------------------------------- StringizeAgent ------------------------------------    

TreePtr<Node> StringizeAgent::BuildNewSubtree(const SCREngine *acting_engine)
{
    PatternLink source_plink(&source);
    TreePtr<Node> new_identifier = source_plink.GetChildAgent()->BuildForBuildersAnalysis(source_plink, acting_engine);
    return MakeTreeNode<CPPTree::SpecificString>( new_identifier->GetRenderTerminal() ); 
}


Syntax::Production StringizeAgent::GetAgentProduction() const
{
	return Syntax::Production::PREFIX;
}


string StringizeAgent::GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const
{
	(void)surround_prod;
	return "§" + kit.render( source, Syntax::Production::PREFIX );
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
