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


Graphable::Block Node::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                          const NonTrivialPreRestrictionFunction &ntprf ) const
{    
	Graphable::Block block;
	block.title = GetGraphName();     
	block.bold = false;
	block.shape = "plaintext";
    block.block_type = Graphable::NODE;
    TreePtr<Node> sp_this( const_pointer_cast<Node>( shared_from_this() ) );
        
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
                link.child = dynamic_cast<Graphable *>(p.get());
                link.style = Graphable::SOLID;
                link.trace_labels.push_back( lnf( &sp_this, &p ) );
                link.is_ntpr = ntprf ? ntprf(&p) : false;
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
                link.child = dynamic_cast<Graphable *>(p.get());
                link.style = Graphable::SOLID;                
                link.trace_labels.push_back( lnf( &sp_this, &p ) );
                link.is_ntpr = ntprf ? ntprf(&p) : false;
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
                link.child = dynamic_cast<Graphable *>(ptr->get());
                link.style = Graphable::SOLID;                
                link.trace_labels.push_back( lnf(&sp_this, ptr ) );          
                link.is_ntpr = ntprf ? ntprf(ptr) : false;
                sub_block.links.push_back( link );
                block.sub_blocks.push_back( sub_block );
   		    }
            else if( ReadArgs::graph_trace )
			{
                Graphable::SubBlock sub_block = { GetInnermostTemplateParam(ptr->GetName()), 
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


string Node::GetGraphId() const
{
    return GetSerialString();
} 
