#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include "walk.hpp"
#include "transformation.hpp"

class Validate : public InPlaceTransformation
{
public:
	Validate( bool p=false ) :
		is_pattern(p)
	{
	}

	virtual void operator()( SharedPtr<Node> context,
	    		             SharedPtr<Node> *proot );
private:
	void OnLink( SharedPtr<Node> p, SharedPtr<Node> c );

	bool is_pattern;
	Map< SharedPtr<Node>, unsigned > decl_refs;
	Map< SharedPtr<Node>, unsigned > total_refs;
};

// TODO
// - All expressions to be TypeOf()-able

#endif
