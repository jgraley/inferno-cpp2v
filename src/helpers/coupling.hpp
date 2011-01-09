#ifndef COUPLING_HPP
#define COUPLING_HPP

#include "common/common.hpp"
#include "tree/generics.hpp"
#include <set>

// This just serves to complicate matters - just use bools directly
// TODO also dosen't belong in couplings lol
enum Result { NOT_FOUND = (int)false,
			  FOUND     = (int)true };


// Base class for coupling keys; this deals with individual node matches, and also with stars
// by means of pointing "root" at a SubCollection or SubSequence
struct Key
{
	virtual ~Key(){}  // be a virtual hierarchy
	TreePtr<Node> root; // Tree node at matched pattern; root of any replace subtree
	TreePtr<Node> replace_pattern; // Tree node at matched pattern; root of any replace subtree TODO fix comment
};

typedef TreePtr<Node> Coupling;
typedef Set<Coupling> CouplingSet;

class CompareReplace;
class CouplingKeys
{
public:
	CouplingKeys()
	{
	}
	Result KeyAndRestrict( TreePtr<Node> x,
						   TreePtr<Node> pattern,
						   const CompareReplace *sr,
						   bool can_key );
	Result KeyAndRestrict( shared_ptr<Key> key,
						   TreePtr<Node> pattern,
						   const CompareReplace *sr,
						   bool can_key );
	TreePtr<Node> KeyAndSubstitute( TreePtr<Node> x, // source after soft nodes etc
									TreePtr<Node> pattern, // source
									const CompareReplace *sr,
									bool can_key );
	TreePtr<Node> KeyAndSubstitute( shared_ptr<Key> key,
									TreePtr<Node> pattern,
									const CompareReplace *sr,
									bool can_key );
private:
	Map< Coupling, shared_ptr<Key> > keys_map;
};


inline CouplingSet operator,( Coupling l, Coupling r )
{
	CouplingSet cs;
    cs.insert( l );
    cs.insert( r );
    return cs;
}


inline CouplingSet operator,( CouplingSet l, Coupling r )
{
    l.insert( r );
    return l;
}

#endif
