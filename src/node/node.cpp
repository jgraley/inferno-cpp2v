/*
 *  Created on: 14 Jun 2009
 *      Author: jgraley
 */

#include "node.hpp"
#include "specialise_oostd.hpp"
#include "common/read_args.hpp"

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


Graphable::Block Node::GetGraphBlockInfo() const
{    
	Graphable::Block block;
	block.title = GetGraphName();     
	block.bold = false;
	block.shape = "plaintext";
    block.block_type = Graphable::NODE_EXPANDED;
    TreePtr<Node> sp_this( const_pointer_cast<Node>( shared_from_this() ) );
    block.node = sp_this;
        
    vector< Itemiser::Element * > members = Itemise();
	for( int i=0; i<members.size(); i++ )
	{
		if( SequenceInterface *seq = dynamic_cast<SequenceInterface *>(members[i]) )
            block.sub_blocks = block.sub_blocks + GetSubblocks(seq);
		else if( CollectionInterface *col = dynamic_cast<CollectionInterface *>(members[i]) )
            block.sub_blocks = block.sub_blocks + GetSubblocks(col);
		else if( TreePtrInterface *singular = dynamic_cast<TreePtrInterface *>(members[i]) )
            block.sub_blocks = block.sub_blocks + GetSubblocks(singular);
		else
			ASSERT(0);
	}
    
    return block;
}


list<Graphable::SubBlock> Node::GetSubblocks( SequenceInterface *seq, 
                                              Phase phase )
{
    list<SubBlock> sub_blocks;
    int j=0;
    FOREACH( const TreePtrInterface &p, *seq )
    {
        Graphable::SubBlock sub_block = { GetInnermostTemplateParam(seq->GetName()), 
                                          SSPrintf("[%d]", j++),
                                          false,
                                          {} };                                                  
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                                                  list<string>{},
                                                  list<string>{},
                                                  phase,
                                                  &p );
        sub_block.links.push_back( link );
        sub_blocks.push_back( sub_block );
    }   
    return sub_blocks; 
}


list<Graphable::SubBlock> Node::GetSubblocks( CollectionInterface *col, 
                                              Phase phase )
{
    list<SubBlock> sub_blocks;
    string dots;
    for( int j=0; j<col->size(); j++ )
        dots += ".";
    
    Graphable::SubBlock sub_block = { GetInnermostTemplateParam(col->GetName()), 
                                      "{" + dots + "}",
                                      false,
                                      {} };
    FOREACH( const TreePtrInterface &p, *col )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                                                  list<string>{},
                                                  list<string>{},
                                                  phase,
                                                  &p );
        sub_block.links.push_back( link );
    }
    sub_blocks.push_back( sub_block );
    return sub_blocks; 
}


list<Graphable::SubBlock> Node::GetSubblocks( const TreePtrInterface *singular, 
                                              Phase phase )
{
    list<SubBlock> sub_blocks;
    if( *singular )
    {
        Graphable::SubBlock sub_block = { GetInnermostTemplateParam(singular->GetName()), 
                                          "",
                                          false,
                                          {} };
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(singular->get()),
                                                  list<string>{},
                                                  list<string>{},
                                                  phase,
                                                  singular );
        sub_block.links.push_back( link );
        sub_blocks.push_back( sub_block );
    }
    else if( ReadArgs::graph_trace )
    {
        Graphable::SubBlock sub_block = { GetInnermostTemplateParam(singular->GetName()), 
                                          "NULL",
                                          false, 
                                          {} };
        sub_blocks.push_back( sub_block );
    }
    return sub_blocks; 
}


string Node::GetGraphId() const
{
    return GetSerialString();
} 


string Node::GetTrace() const
{
    return Traceable::GetName() + GetSerialString();
}
