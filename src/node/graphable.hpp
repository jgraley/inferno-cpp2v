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
    struct Link : Traceable
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
        string GetTrace() const override;

        const Graphable *child;
        list<string> labels;
        list<string> trace_labels;
        Phase phase;
        const TreePtrInterface *pptr;
    };
    struct SubBlock : Traceable
    {
		SubBlock( string item_name_,
				  string item_extra_,
				  bool hideable_,
				  list< shared_ptr<Link> > links_ ) :
			item_name( item_name_ ),
			item_extra( item_extra_ ),
			hideable( hideable_ ),
			links( links_ )
		{
		}
		string GetTrace() const override;
        
        string item_name;
        string item_extra;
        bool hideable;
        list< shared_ptr<Link> > links;
    };
    struct Block : Traceable
    {
		Block() {} // TODO pernsion this off
		Block( bool bold_, // TODO reorder and add defaults
			   string title_,
			   string symbol_,
			   string shape_,
			   BlockType block_type_,
			   shared_ptr<const Node> node_,
			   list<SubBlock> sub_blocks_ ) :
			bold( bold_ ),
			title( title_ ),
			symbol( symbol_ ),
			shape( shape_ ),
			block_type( block_type_ ),
			node( node_ ),
			sub_blocks( sub_blocks_ )
		{
		}
			   
        string GetTrace() const override;

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
    
    // If you don't know about pre-restrictions, you don't have one.
    virtual bool IsNonTrivialPreRestrictionNP(const TreePtrInterface *) const { return false; }
};

string Trace(const GraphIdable::Phase &phase);
string Trace(const Graphable::BlockType &type);

#endif
