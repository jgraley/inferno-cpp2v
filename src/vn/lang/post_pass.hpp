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
    void ProcessNode( TreePtr<Node> x, DData dd );
    void ProcessChildren( TreePtr<Node> x, DData dd );
    void ProcessSingularItem( TreePtr<Node> x, TreePtrInterface *p_x_sing, DData dd );
    void ProcessSequence( TreePtr<Node> x, SequenceInterface *x_seq, DData dd );
    void ProcessCollection( TreePtr<Node> x, CollectionInterface *x_col, DData dd );    
    
private:
};

};

#endif
