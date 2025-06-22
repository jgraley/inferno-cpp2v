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
    auto n = make_shared<MyNode>();
    TRACE();
    vector< Itemiser::Element * > i = n->Itemise();
    TRACE();
    //for( int e=0; e<i.size(); e++ )
    //    TRACE("element at %p\n", i[e]);
    ASSERT( i.size() == 1 )("%d elements\n", i.size());
}


Graphable::NodeBlock Node::GetGraphBlockInfo() const
{    
    Graphable::NodeBlock block;
    block.title = GetGraphName();     
    block.bold = false;
    block.shape = "plaintext";
    block.block_type = Graphable::NODE_EXPANDED;
    TreePtr<Node> sp_this( const_pointer_cast<Node>( shared_from_this() ) );
    block.node = sp_this;
        
    vector< Itemiser::Element * > members = Itemise();
    for( vector< Itemiser::Element * >::size_type i=0; i<members.size(); i++ )
    {
        if( SequenceInterface *seq = dynamic_cast<SequenceInterface *>(members[i]) )
            block.item_blocks = block.item_blocks + GetSubblocks(seq);
        else if( CollectionInterface *col = dynamic_cast<CollectionInterface *>(members[i]) )
            block.item_blocks = block.item_blocks + GetSubblocks(col);
        else if( TreePtrInterface *singular = dynamic_cast<TreePtrInterface *>(members[i]) )
            block.item_blocks = block.item_blocks + GetSubblocks(singular);
        else
            ASSERT(0);
    }
    
    return block;
}


list<Graphable::ItemBlock> Node::GetSubblocks( SequenceInterface *seq, 
                                              Phase phase )
{
    list<ItemBlock> item_blocks;
    int j=0;
    for( const TreePtrInterface &p : *seq )
    {
        Graphable::ItemBlock sub_block = { GetInnermostTemplateParam(seq->GetName()), 
                                          SSPrintf("[%d]", j++),
                                          false,
                                          {} };                                                  
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                                                  list<string>{},
                                                  list<string>{},
                                                  phase,
                                                  &p );
        sub_block.links.push_back( link );
        item_blocks.push_back( sub_block );
    }   
    return item_blocks; 
}


list<Graphable::ItemBlock> Node::GetSubblocks( CollectionInterface *col, 
                                              Phase phase )
{
    list<ItemBlock> item_blocks;
    string dots;
    for( int j=0; j<col->size(); j++ )
        dots += ".";
    
    Graphable::ItemBlock sub_block = { GetInnermostTemplateParam(col->GetName()), 
                                      "{" + dots + "}",
                                      false,
                                      {} };
    for( const TreePtrInterface &p : *col )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                                                  list<string>{},
                                                  list<string>{},
                                                  phase,
                                                  &p );
        sub_block.links.push_back( link );
    }
    item_blocks.push_back( sub_block );
    return item_blocks; 
}


list<Graphable::ItemBlock> Node::GetSubblocks( const TreePtrInterface *singular, 
                                              Phase phase )
{
    list<ItemBlock> item_blocks;
    if( *singular )
    {
        Graphable::ItemBlock sub_block = { GetInnermostTemplateParam(singular->GetName()), 
                                          "",
                                          false,
                                          {} };
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(singular->get()),
                                                  list<string>{},
                                                  list<string>{},
                                                  phase,
                                                  singular );
        sub_block.links.push_back( link );
        item_blocks.push_back( sub_block );
    }
    else if( ReadArgs::graph_trace )
    {
        Graphable::ItemBlock sub_block = { GetInnermostTemplateParam(singular->GetName()), 
                                          "NULL",
                                          false, 
                                          {} };
        item_blocks.push_back( sub_block );
    }
    return item_blocks; 
}


string Node::GetGraphId() const
{
    return GetSerialString();
} 


string Node::GetTrace() const
{
    return GetName() + GetSerialString();
}
