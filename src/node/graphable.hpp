#ifndef GRAPHABLE_HPP
#define GRAPHABLE_HPP

#include "common/common.hpp"
#include "common/trace.hpp"

#include <string>
#include <list>
#include <functional>

using namespace std;

class TreePtrInterface;
template<typename VALUE_TYPE>
struct TreePtr;
class Node;

class Graphable : public virtual Traceable
{
public:    

    enum LinkStyle
    {
        LINK_NORMAL,
        LINK_ROOT, // Like LINK_NORMAL but disappears in And-rule graphs
        LINK_KEYER,
        LINK_RESIDUAL,
        LINK_ONLY_REPLACE, // Disappears in And-rule graphs
        LINK_ABNORMAL,
        LINK_EVALUATOR,
        LINK_MULTIPLICITY        
    };
    enum BlockType
    {
        CONTROL,
        NODE    
    };
    struct Link
    {
        const Graphable *child;
        LinkStyle style;
        list<string> labels;
        list<string> trace_labels;
        bool is_ntpr;
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
    
    typedef std::function<string( const TreePtr<Node> *parent_pattern,
                                  const TreePtrInterface *ppattern )> LinkNamingFunction;
    typedef std::function<bool( const TreePtrInterface *ppattern )> NonTrivialPreRestrictionFunction;
                                  
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const     
    { 
        Block g{false, "", "", "", NODE, {}}; 
        return g;
    };
    virtual string GetGraphId() const { return ""; }
};

#endif
