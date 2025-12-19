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
#include "tree/node_names.hpp"
#include <any> // to dep-break the generated headers

namespace YY
{
	class VNLangScanner;
	class VNLangParser;
};

namespace VN 
{
		
struct Limit
{
	string cond;
	any cond_loc;
	string num_text;
	any num_loc;
};
		
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
class VNLangRecogniser;

class VNParse	
{
public:
	VNParse();
	~VNParse();
	
	Command::List DoParse(string filepath);	 
	
	void OnError();
	void OnVNScript( list<shared_ptr<Command>> commands_ );
	shared_ptr<Command> OnCommand( shared_ptr<Command> command );

	TreePtr<Node> OnStar( TreePtr<Node> restriction );
	TreePtr<Node> OnStuff( TreePtr<Node> terminus, TreePtr<Node> recurse_restriction, Limit limit );
	TreePtr<Node> OnDelta( TreePtr<Node> through, TreePtr<Node> overlay );
	TreePtr<Node> OnBuiltIn( const AvailableNodeData::Block *block, any node_name_loc, Itemisation itemisation );
	TreePtr<Node> OnEmbeddedCommands( list<shared_ptr<Command>> commands );
	TreePtr<Node> OnRestrict( const AvailableNodeData::Block *block, any node_name_loc, TreePtr<Node> target, any target_loc );
	
	TreePtr<Node> OnInfixOperator( string tok, TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnPrefixOperator( string tok, TreePtr<Node> operand );
	TreePtr<Node> OnPostfixOperator( string tok, TreePtr<Node> operand );
	TreePtr<Node> OnIntegralLiteral( string text, any loc ); 
	TreePtr<Node> OnStringLiteral( wstring value );
	TreePtr<Node> OnBoolLiteral( bool value );
	TreePtr<Node> OnCast( TreePtr<Node> type, any type_loc, TreePtr<Node> target, any target_loc );	
	TreePtr<Node> OnConditionalOperator( TreePtr<Node> condition, TreePtr<Node> expr_then, TreePtr<Node> expr_else );	
	TreePtr<Node> OnSubscript( TreePtr<Node> destination, TreePtr<Node> index );	
	TreePtr<Node> OnCompound( list<TreePtr<Node>> statements );	
	TreePtr<Node> OnArrayLiteral( Item elements );	
	
	TreePtr<Node> OnNormalTerminalKeyword( string keyword, any keyword_loc );
	TreePtr<Node> OnSpaceSepStmtKeyword( string keyword, any keyword_loc, TreePtr<Node> operand, any operand_loc );
	
	TreePtr<Node> OnIdValuePair( TreePtr<Node> id, any id_loc, TreePtr<Node> value );
	TreePtr<Node> OnMapArgsCall( TreePtr<Node> callee, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnSeqArgsCall( TreePtr<Node> callee, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnLookup( TreePtr<Node> object, TreePtr<Node> member, any member_loc );

	TreePtr<Node> OnSpecificId( const AvailableNodeData::Block *block, any id_disc_loc, wstring wname, any name_loc );
	TreePtr<Node> OnIdByName( const AvailableNodeData::Block *block, any id_disc_loc, wstring wname, any name_loc );
	TreePtr<Node> OnBuildId( const AvailableNodeData::Block *block, any id_disc_loc, wstring wformat, any name_loc, Item sources );
	TreePtr<Node> OnTransform( string kind, any kind_loc, TreePtr<Node> pattern, any pattern_loc );
	TreePtr<Node> OnNegation( TreePtr<Node> operand );
	TreePtr<Node> OnConjunction( TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnDisjunction( TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnGrass( TreePtr<Node> through );
	TreePtr<Node> OnPointerIs( TreePtr<Node> pointer );
	TreePtr<Node> OnBuildSize( TreePtr<Node> container );
	TreePtr<Node> OnStringize( TreePtr<Node> source );
	
	TreePtr<Node> CreateIntegralLiteral( bool uns, bool lng, bool lng2, uint64_t val, any loc );
	VNLangRecogniser &GetShim();
	
private: 
	unique_ptr<VNLangRecogniser> recogniser;
	unique_ptr<YY::VNLangScanner> scanner;
	unique_ptr<YY::VNLangParser> parser;
	unique_ptr<AvailableNodeData> node_names;
	
	Command::List top_level_commands;
};
	
};

#endif

