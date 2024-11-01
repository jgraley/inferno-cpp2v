#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

class Validate
{
public:
	Validate( bool p=false ) :
		is_pattern(p)
	{
	}

	virtual void operator()( TreePtr<Node> root, 
                             TreePtr<Node> context = TreePtr<Node>() );
    
private:
	void OnLink( TreePtr<Node> p, TreePtr<Node> c );

	bool is_pattern;
	map< TreePtr<Node>, unsigned > decl_refs;
	map< TreePtr<Node>, unsigned > total_refs;
};

// TODO
// - All expressions to be TypeOf()-able

#endif
