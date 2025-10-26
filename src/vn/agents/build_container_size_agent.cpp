#include "build_container_size_agent.hpp"
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


Syntax::Production BuildContainerSizeAgent::GetAgentProduction() const
{
	return Syntax::Production::PREFIX;
}


string BuildContainerSizeAgent::GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const
{
	(void)surround_prod;
	string s = "⧆" + kit.render( container, Syntax::Production::PREFIX );
	return s;
} 
  
    
string BuildContainerSizeAgent::GetCouplingNameHint() const
{
	return "size";
} 


bool BuildContainerSizeAgent::IsNonTrivialPreRestriction(const TreePtrInterface *) const
{
    return false;
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

