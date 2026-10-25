#include "build_container_size_agent.hpp"
#include "tree/cpptree.hpp"
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
    ASSERT( container.size()==1 ); // TODO could have bigger, but would need to change VN syntax which is currently prefix
    PatternLink container_plink(&container.front());
    TreePtr<Node> new_node = container_plink.GetChildAgent()->BuildForBuildersAnalysis(container_plink, acting_engine);
    ASSERT( new_node );
    ContainerInterface *new_container = dynamic_cast<ContainerInterface *>(new_node.get());
    ASSERT( new_container );
    int size = new_container->size();
    return MakeTreeNode<SpecificInteger>(size); 
}                                                   


Syntax::Production BuildContainerSizeAgent::GetAgentProduction( const VN::RendererInterface *, Syntax::Policy ) const
{
	return Syntax::Production::PREFIX;
}


string BuildContainerSizeAgent::GetAgentRender( VN::RendererInterface *renderer, Syntax::Production surround_prod, Syntax::Policy policy ) const
{
    ASSERT( container.size()==1 ); // TODO could have bigger, but would need to change VN syntax which is currently prefix
	const TreePtr<Node> &cont_node = const_cast<Collection<Node> &>(container).front();
	(void)surround_prod;
	string s = "🄽" + renderer->DoRenderPreserve( cont_node, Syntax::Production::PREFIX, policy );
	return s;
} 
  
    
string BuildContainerSizeAgent::GetDesignationNameHint() const
{
	return "size";
} 


bool BuildContainerSizeAgent::IsFixedType() const
{
    return true;
} 


Graphable::NodeBlock BuildContainerSizeAgent::GetGraphBlockInfo() const
{
    NodeBlock block;
    block.bold = false;
    block.title = GetName();
    block.shape = "egg";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.item_blocks = { { "container", 
                           "", 
                           false,
                           {} } };
    for( const TreePtrInterface &p : const_cast<Collection<Node> &>(container) )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &p );
        block.item_blocks.front().links.push_back( link );
    }
    return block;
}

