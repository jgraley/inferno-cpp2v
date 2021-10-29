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
              bool is_nontrivial_prerestriction_ ) :
            child( child_ ),
            labels( labels_ ),
            trace_labels( trace_labels_ ),
            phase( phase_ ),
            is_nontrivial_prerestriction( is_nontrivial_prerestriction_ )
        {
        }
               
        Link( const Link &other ) : 
            Link( other.child, other.labels, other.trace_labels, other.phase, other.is_nontrivial_prerestriction )
        {
        }
               
        virtual ~Link() = default;

        const Graphable *child;
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
        list< shared_ptr<Link> > links;
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
        Block g{false, "", "", "", NODE_SHAPED, {}}; 
        return g;
    };
};

#endif
