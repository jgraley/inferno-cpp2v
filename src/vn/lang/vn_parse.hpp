#ifndef VN_PARSE_HPP
#define VN_PARSE_HPP

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

namespace VN 
{
typedef TreePtr<Node> Production;	
 
class VNParse	
{
public:
	VNParse();
	~VNParse();
	
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
	Production OnRestrict( any loc, wstring type, Production target ) {(void)loc; (void)type;return target;};
	Production OnRestrict( list<string> res_type, any res_loc, Production target, any target_loc );
	
	Production OnPrefixOperator( string tok, Production operand );
	Production OnSpecificInteger( int value );

private: 
	unique_ptr<YY::VNLangScanner> scanner;
	unique_ptr<YY::VNLangParser> parser;
	
	bool saw_error;
	TreePtr<Node> script;	
};
	
	
};

#endif

