#ifndef GRAPHABLE_HPP
#define GRAPHABLE_HPP

#include "common/common.hpp"
#include "common/trace.hpp"
#include "tree_ptr.hpp"

#include <string>
#include <list>
#include <functional>

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
        LinkStyle default_link_style;
        list<SubBlock> sub_blocks;
    };

    typedef std::function<string( shared_ptr<const Node> parent_pattern,
                                  const TreePtrInterface *ppattern )> LinkNamingFunction;
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf ) const 
    { 
        Block g{false, "", "", "", NODE, {}}; 
        return g;
    };
    
    virtual string GetGraphId() const { return ""; }
};

#endif
