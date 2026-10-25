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
	const TreePtr<Node> &cont_node = const_cast<Collection<Node> &>(container).front();
    NodeBlock block;
    block.bold = false;
    block.title = GetName();
    block.shape = "egg";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    if( cont_node )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(cont_node.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &cont_node );
        block.item_blocks.push_back( { "container", 
                                      "", 
                                      false,
                                      { link } } );
    }
    return block;
}

