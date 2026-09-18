#ifndef POST_PASS_HPP
#define POST_PASS_HPP

#include <map>
#include <vector>
#include "common/standard.hpp"
#include "helpers/simple_compare.hpp"
#include "helpers/transformation.hpp"
#include "db/mutator.hpp"

namespace VN 
{

class PostPass
{
public:
    PostPass();    
	TreePtr<Node> Run( TreePtr<Node> root );
	
private:
	struct DData // passed by value to descendants
	{
	};
	
	void ProcessMutator( Mutator mutator, DData dd );
    void ProcessNode( TreePtr<Node> old_x, TreePtr<Node> new_x, DData dd );
    void ProcessChildren( TreePtr<Node> old_x, TreePtr<Node> new_x, DData dd );
    void ProcessSingularItem( TreePtr<Node> new_x, TreePtrInterface *old_p_x_sing, TreePtrInterface *new_p_x_sing, DData dd );
    void ProcessSequence( TreePtr<Node> new_x, SequenceInterface *old_x_seq, SequenceInterface *new_x_seq, DData dd );
    void ProcessCollection( TreePtr<Node> new_x, CollectionInterface *old_x_col, CollectionInterface *new_x_col, DData dd );    
	
	map<TreePtr<Node>, TreePtr<Node>> changes;
};

};

#endif
