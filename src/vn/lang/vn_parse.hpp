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
typedef TreePtr<Node> Production;	
 
class VNParse	
{
public:
	TreePtr<Node> DoParse(string filepath);
	
	struct PureEngine : Node
	{
		NODE_FUNCTIONS_FINAL
		TreePtr<Node> stem;
	};	 
	
	void OnError();
	void OnVNScript( Production script_ );
	Production OnEngine( Production stem );
	Production OnStuff( Production terminus );
	Production OnDelta( Production through, Production overlay );
	
	Production OnPrefixOperator( string tok, Production operand );
	Production OnSpecificInteger( int value );

private: 
	bool saw_error;
	TreePtr<Node> script;	
};
	
	
};

#endif

