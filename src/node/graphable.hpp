#ifndef GRAPHABLE_HPP
#define GRAPHABLE_HPP

#include "common/common.hpp"
#include "common/trace.hpp"
#include "tree_ptr.hpp"

#include <string>
#include <list>

using namespace std;

class Graphable : public virtual Traceable
{
public:    

    enum LinkStyle
    {
        SOLID,
        DASHED,
        THROUGH // means "inherit from parent"
    };
    enum BlockType
    {
        CONTROL,
        NODE    
    };
    struct Link
    {
        TreePtr<Node> child_node;
        const TreePtrInterface *ptr;
        LinkStyle link_style;
        list<string> labels;
        list<string> trace_labels;
    };
    struct SubBlock
    {
        string item_name;
        string item_extra;
        bool hideable;
        list<Link> links;
    };
    struct Block
    {
        bool bold;
        string title;
        string symbol;
        string shape;
        BlockType block_type;
        list<SubBlock> sub_blocks;
    };
    virtual Block GetGraphBlockInfo() const 
    { 
        Block g{false, "", "", "", NODE, {}}; 
        return g;
    };
};

#endif
