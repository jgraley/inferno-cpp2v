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

class GraphIdable : public virtual Traceable
{
public:    
    enum Phase
    {
        // Really a bitfield
        UNDEFINED = 0,
        IN_COMPARE_ONLY = 1,
        IN_COMPARE_AND_REPLACE = 3,
        IN_REPLACE_ONLY = 2,
    };

    virtual string GetGraphId() const { return ""; }
};

class Graphable : public GraphIdable
{
public:    
    enum BlockType
    {
        CONTROL,
        NODE_SHAPED,
        NODE_EXPANDED,
        INVISIBLE    
    };
    struct Link
    {
        Link( const Graphable *child_,
              list<string> labels_,
              list<string> trace_labels_,
              Phase phase_,
              const TreePtrInterface *pptr_ ) :
            child( child_ ),
            labels( labels_ ),
            trace_labels( trace_labels_ ),
            phase( phase_ ),
            pptr( pptr_ )
        {
        }
               
        Link( const Link &other ) : 
            Link( other.child, 
                  other.labels, 
                  other.trace_labels, 
                  other.phase, 
                  other.pptr )
        {
        }
               
        virtual ~Link() = default;

        const Graphable *child;
        list<string> labels;
        list<string> trace_labels;
        Phase phase;
        const TreePtrInterface *pptr;
    };
    struct SubBlock
    {
        string item_name;
        string item_extra;
        bool hideable;
        list< shared_ptr<Link> > links;
    };
    struct Block
    {
        bool bold;
        string title;
        string symbol;
        string shape;
        BlockType block_type;
        shared_ptr<const Node> node;
        list<SubBlock> sub_blocks;
    };
                                      
    virtual Block GetGraphBlockInfo() const     
    { 
        Block g{false, "", "", "", NODE_SHAPED, nullptr, {}}; 
        return g;
    };
};

#endif
