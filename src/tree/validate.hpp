#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

class Validate : public InPlaceTransformation
{
public:
	Validate( bool p=false ) :
		is_pattern(p)
	{
	}

	virtual void operator()( TreePtr<Node> context,
	    		             TreePtr<Node> *proot );
private:
	void OnLink( TreePtr<Node> p, TreePtr<Node> c );

	bool is_pattern;
	map< TreePtr<Node>, unsigned > decl_refs;
	map< TreePtr<Node>, unsigned > total_refs;
};

// TODO
// - All expressions to be HasType()-able

#endif
