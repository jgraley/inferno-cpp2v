#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include "walk.hpp"
#include "transformation.hpp"

class Validate : public Transformation
{
public:
	Validate( bool p=false ) :
		is_pattern(p)
	{
	}

	virtual void operator()( shared_ptr<Node> context,
	    		             shared_ptr<Node> *proot );
private:
	void OnLink( shared_ptr<Node> p, shared_ptr<Node> c );

	bool is_pattern;
	Map< shared_ptr<Node>, unsigned > decl_refs;
	Map< shared_ptr<Node>, unsigned > total_refs;
};

// TODO
// - All nodes final except in patterns
// - loop detection (positive check, not just spin forever)
// - All expressions to be TypeOf()-able

#endif
