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
	
typedef TreePtr<Node> TreePtr<Node>;
	
struct Item
{
	list<TreePtr<Node>> nodes;
	any loc;
};

struct Itemisation
{
	list<Item> items;
	any loc;
};


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

	TreePtr<Node> OnStuff( TreePtr<Node> terminus );
	TreePtr<Node> OnDelta( TreePtr<Node> through, TreePtr<Node> overlay );
	TreePtr<Node> OnBuiltIn( list<string> builtin_type, any builtin_loc, Itemisation itemisation );
	TreePtr<Node> OnName( wstring name, any name_loc );
	TreePtr<Node> OnEmbeddedCommands( list<shared_ptr<Command>> commands );
	TreePtr<Node> OnRestrict( list<string> res_type, any res_loc, TreePtr<Node> target, any target_loc );
	
	TreePtr<Node> OnPrefixOperator( string tok, TreePtr<Node> operand );
	TreePtr<Node> OnPostfixOperator( string tok, TreePtr<Node> operand );
	TreePtr<Node> OnInfixOperator( string tok, TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnSpecificInteger( int value );

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

