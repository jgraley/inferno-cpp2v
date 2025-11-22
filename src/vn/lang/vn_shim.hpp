#ifndef VN_SHIM_HPP
#define VN_SHIM_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"
#include <any> // to dep-break the generated headers

// WHAT WE NEED
// A "name info" struct containing a variety of handy info relating to a name we saw:
// - unicode/ASCII flag
// - A subtree if the name was designated
// - An archetype if the type is known
// - An enum of TYPE, OTHER etc which has been deduced from the above according to some priority scheme
// It is this enum that the scanner will use to decide what token to issue
// Alternatively, WE could choose the token and put that in the struct instead.

namespace YY
{
	class VNLangScanner;
	class VNLangParser;
};

class NodeNames;

namespace VN 
{
	
class VNParse;	
	
class VNShim
{
public:
	VNShim( const VNParse *parse_ );
	
	//bool IsType() const;
	TreePtr<Node> TryGetNamedSubtree(wstring name) const;	
	TreePtr<Node> TryGetArchetype( list<string> typ ) const;

private:	
	const VNParse * const parse;
	
};
	
};

#endif

