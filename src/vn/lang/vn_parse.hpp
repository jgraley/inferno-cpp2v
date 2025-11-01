#ifndef VN_PARSE_HPP
#define VN_PARSE_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"

namespace VN 
{
//typedef TreePtr<Node> Production;	
typedef float Production;	
 
class VNParse	
{
public:
	void DoParse(string filepath);
	
	Production OnEngine( Production stem );
};
	
	
};

#endif

