#include "graphable.hpp"

#include "common/common.hpp"
#include "common/trace.hpp"
#include "tree_ptr.hpp"

#include <string>
#include <list>
#include <functional>

string Graphable::Link::GetTrace() const
{
	return string("(") +
		   "child:" + Trace(child) + ", " + 
		   "labels:" + Trace(labels) + ", " + 
		   "trace_labels:" + Trace(trace_labels) + ", " + 
		   "phase:" + Trace(phase) + ", " + 
		   "pptr:" + Trace(pptr) +
		   ")";
}


string Graphable::SubBlock::GetTrace() const
{
	return string("(") +
		   "item_name:" + Trace(item_name) + ", " + 
		   "item_extra:" + Trace(item_extra) + ", " + 
		   "hideable:" + Trace(hideable) + ", " + 
		   "links:" + Trace(links) +
		   ")";    
}


string Graphable::Block::GetTrace() const 
{
	return string("(") +
		   "bold:" + Trace(bold) + ", " + 
		   "title:" + Trace(title) + ", " + 
		   "symbol:" + Trace(symbol) + ", " + 
		   "shape:" + Trace(shape) +  ", " + 
           "block_type:" + Trace(block_type) +  ", " + 
		   "node:" + Trace(node) + ", " +
		   "sub_blocks:" + Trace(sub_blocks) +
		   ")";    
}  


string Trace(const GraphIdable::Phase &phase)
{
	switch( phase )
	{
		case GraphIdable::Phase::UNDEFINED:
			return "UNDEFINED";
			
		case GraphIdable::Phase::IN_COMPARE_ONLY:
			return "IN_COMPARE_ONLY";
			
        case GraphIdable::Phase::IN_COMPARE_AND_REPLACE:
			return "IN_COMPARE_AND_REPLACE";
			
		case GraphIdable::Phase::IN_REPLACE_ONLY:
			return "IN_REPLACE_ONLY";
	}
	return "???";
}


string Trace(const Graphable::BlockType &type)
{
	switch( type )
	{
		case Graphable::BlockType::CONTROL:
			return "CONTROL";
			
		case Graphable::BlockType::NODE_SHAPED:
			return "NODE_SHAPED";
			
        case Graphable::BlockType::NODE_EXPANDED:
			return "NODE_EXPANDED";
			
		case Graphable::BlockType::INVISIBLE:
			return "INVISIBLE";
	}
	return "???";
}


