#ifndef VN_PARSE_HPP
#define VN_PARSE_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"
#include "vn_commands.hpp"
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
typedef TreePtr<Node> Production;	
class Command;

class VNParse	
{
public:
	VNParse();
	~VNParse();
	
	Command::List DoParse(string filepath);	 
	
	void OnError();
	void OnVNScript( list<shared_ptr<Command>> commands_ );
	shared_ptr<Command> OnCommand( shared_ptr<Command> command );

	Production OnEngine( Production stem );
	Production OnStuff( Production terminus );
	Production OnDelta( Production through, Production overlay );
	Production OnBuiltIn( list<string> builtin_type, any builtin_loc, list<list<Production>> itemisation );
	Production OnName( wstring name, any name_loc );
	Production OnRestrict( list<string> res_type, any res_loc, Production target, any target_loc );
	
	Production OnPrefixOperator( string tok, Production operand );
	Production OnPostfixOperator( string tok, Production operand );
	Production OnInfixOperator( string tok, Production left, Production right );
	Production OnSpecificInteger( int value );

	void Designate( wstring name, TreePtr<Node> sub_pattern );
	
private: 
	unique_ptr<YY::VNLangScanner> scanner;
	unique_ptr<YY::VNLangParser> parser;
	unique_ptr<NodeNames> node_names;
	
	bool saw_error;
	Command::List commands;
	map<wstring, TreePtr<Node>> designations;
};
	
};

#endif

