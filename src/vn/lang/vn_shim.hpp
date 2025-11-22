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

namespace YY
{
	class VNLangScanner;
	class VNLangParser;
};

// Dep-break NodeNames because node_names.hpp will be big
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

