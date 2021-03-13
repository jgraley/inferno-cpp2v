/*
 *  Created on: 14 Jun 2009
 *      Author: jgraley
 */

#include "node.hpp"
#include "specialise_oostd.hpp"

struct MyNode : Node
{
	NODE_FUNCTIONS
	Sequence<Node> s;
};

void GenericsTest()
{
	TRACE();
    shared_ptr<MyNode> n( new MyNode );
	TRACE();
    vector< Itemiser::Element * > i = n->Itemise();
	TRACE();
    //for( int e=0; e<i.size(); e++ )
    //    TRACE("element at %p\n", i[e]);
    ASSERT( i.size() == 1 )("%d elements\n", i.size());
}

#ifdef NODE_IS_GRAPHABLE
Graphable::Block Node::GetGraphBlockInfo( const LinkNamingFunction &lnf )
{    
	Graphable::Block block;
	block.title = GetGraphName();     
	block.bold = false;
	block.shape = "plaintext";
        
    vector< Itemiser::Element * > members = Itemise();
	for( int i=0; i<members.size(); i++ )
	{
		if( SequenceInterface *seq = dynamic_cast<SequenceInterface *>(members[i]) )
		{
            int j=0;
			FOREACH( const TreePtrInterface &p, *seq )
			{
                Graphable::SubBlock sub_block = { GetInnermostTemplateParam(seq->GetName()), 
                                                  SSPrintf("[%d]", j++),
                                                  false,
                                                  {} };
                Graphable::Link link;
                link.ptr = &p;
                link.link_style = Graphable::THROUGH;
                link.trace_labels.push_back( lnf( n, &p ) );
                sub_block.links.push_back( link );
                block.sub_blocks.push_back( sub_block );
			}
		}
		else if( CollectionInterface *col = dynamic_cast<CollectionInterface *>(members[i]) )
		{
            string dots;
            for( int j=0; j<col->size(); j++ )
                dots += ".";
            
            Graphable::SubBlock sub_block = { GetInnermostTemplateParam(col->GetName()), 
                                              "{" + dots + "}",
                                              false,
                                              {} };
			FOREACH( const TreePtrInterface &p, *col )
            {
                Graphable::Link link;
                link.ptr = &p;
                link.link_style = Graphable::THROUGH;                
                link.trace_labels.push_back( lnf( n, &p ) );
                sub_block.links.push_back( link );
            }
            block.sub_blocks.push_back( sub_block );
		}
		else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(members[i]) )
		{
			if( *ptr )
			{
                Graphable::SubBlock sub_block = { GetInnermostTemplateParam(ptr->GetName()), 
                                                  "",
                                                  false,
                                                  {} };
                Graphable::Link link;
                link.ptr = ptr;
                link.link_style = Graphable::THROUGH;                
                link.trace_labels.push_back( lnf( n, &p ) );          
                sub_block.links.push_back( link );
                block.sub_blocks.push_back( sub_block );
   		    }
            else if( ReadArgs::graph_trace )
			{
                Graphable::SubBlock sub_block = { EscapeForGraphviz(GetInnermostTemplateParam(ptr->GetName())), 
                                                  "NULL",
                                                  false, 
                                                  {} };
                block.sub_blocks.push_back( sub_block );
            }
		}
		else
		{
			ASSERT(0);
		}
	}
    
    return block;
}
#endif
