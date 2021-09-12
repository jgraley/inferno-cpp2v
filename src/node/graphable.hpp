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
        LINK_KEYER,
        LINK_RESIDUAL,
        LINK_ABNORMAL,
        LINK_EVALUATOR,
        LINK_MULTIPLICITY        
    };
    enum BlockType
    {
        CONTROL,
        NODE    
    };
    enum Phase
    {
        // Really a bitfield
        IN_COMPARE_ONLY = 1,
        IN_COMPARE_AND_REPLACE = 3,
        IN_REPLACE_ONLY = 2
    };
    struct Link
    {
        const Graphable *child;
        LinkStyle style;
        list<string> labels;
        list<string> trace_labels;
        Phase phase;
        bool is_nontrivial_prerestriction;
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
