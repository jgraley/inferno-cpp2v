#include "vn_shim.hpp"

#include "tree/cpptree.hpp"
#include "tree/localtree.hpp"
#include "helpers/transformation.hpp"
#include "tree/typeof.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "tree/type_data.hpp"
#include "helpers/walk.hpp"
#include "helpers/simple_duplicate.hpp"
#include "tree/misc.hpp"
#include "tree/scope.hpp"
#include "sort_decls.hpp"
#include "vn_lang.ypp.hpp"
#include "vn_lang.lpp.hpp"
#include "vn_lang.location.hpp"
#include "vn/agents/all.hpp"
#include "tree/node_names.hpp"
#include "vn_commands.hpp"
#include "vn_parse.hpp"

#include <iostream>
#include <fstream>
#include <cctype>

using namespace CPPTree; // TODO should not need
using namespace VN;
using namespace reflex;


static TreePtr<Node> MakeStandardAgent(NodeEnum ne)
{
	switch(ne)
	{
#define NODE(NS, NAME) \
	case NodeEnum::NS##_##NAME: \
		return MakeTreeNode<StandardAgentWrapper<NS::NAME>>(); 
#include "tree/node_names.inc"			
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "tree/operator_data.inc"
#undef NODE
	}
	
	// By design we should have a case for every value of the node enum
	ASSERT(false)("Invalid value for node enum value %d", ne); 
	ASSERTFAIL();
}

// You want to call 
// YY::VNLangParser::symbol_type YY::VNLangParser::make_SOME_KIND_OF_NAME( string v, location_type l ) <--- a static function
// of if you want the token in a varaible eg tok, use
// YY::VNLangParser::symbol_type::symbol_type( int tok, VNShim::NameData, location_type l )   <--- a constructor
// with tok = token::TOK_SOME_KIND_OF_NAME
// and VNShim::NameData being the type of TOK_SOME_KIND_OF_NAME etc as specified in the .ypp file
//
// Scanner rule is
// xyz   { return shim.OnUnicodeName( wstr(), location() ); }
//
// defined like YY::VNLangParser::symbol_type VNShim::OnUnicodeName( wstring v, location_type l );   <--- or "any" for location
//
// ALL strings found by parser go in here, ASCII or Unicode, quoted or not, including keywords.
// There's a separate entry point for each parsing token. EP for quoted should unquote.
// All call a common analysis function:
// - designations
// - Node type names
// - Indeitifer subtypes
// Returning this data in a struct VNShim::NameData which will also be passed to parser
// Then do a priority-based analysis that leads to a choice of parser token. Presumably, we
// can go top priotity first, doing eg
// if (cond)
//     return YY::VNLangParser::make_SOME_KIND_OF_NAME( string, name_data );
//
// Stored designations: store the PARSED "kind" and recover in shim. Thus, we only need the kind 
// for explicit nodes, and we don't need to go hunting through the pattern. Mis-parses like 
// conjunction( type, not-type ) will cause parse errors.

shared_ptr<Gnomon> VNShim::SetScopeRes( list<string> resolution )
{
	auto spg = make_shared<Gnomon>(resolution);
	current_gnomons.push( spg );
	return spg;
}



void VNShim::Designate( wstring name, TreePtr<Node> sub_pattern )
{
	designations.insert( make_pair(name, sub_pattern) );
}




YY::VNLangParser::symbol_type VNShim::OnUnquoted(string word, YY::VNLangParser::location_type loc) const
{
	return OnWord( ToUnicode(word), false, true, loc );
}


YY::VNLangParser::symbol_type VNShim::OnUnquoted(wstring word, YY::VNLangParser::location_type loc) const
{
	return OnWord( word, false, false, loc );
}


YY::VNLangParser::symbol_type VNShim::OnWord(wstring word, bool quoted, bool ascii, YY::VNLangParser::location_type loc) const
{
	Data data;
	(void)ascii;

	FTRACE(word)(" with ")(current_gnomons)("\n");

	// Where unicode is allowed, ascii is allowed too, so positive checks only
	if( !quoted && designations.count(word) > 0 )	
		data.designated = designations.at(word);
	else
		data.designated = nullptr;
		
	if( data.designated )
         return YY::VNLangParser::make_NAMED_SUBTREE(data.designated, loc);
    else if( ascii )
         return YY::VNLangParser::make_ASCII_NAME(ToASCII(word), loc);
    else
         return YY::VNLangParser::make_UNICODE_NAME(word, loc);
}


TreePtr<Node> VNShim::TryGetArchetype( list<string> typ ) const
{
	if( NodeNames().GetNameToEnumMap().count(typ) > 0 )
	{		
		NodeEnum ne = NodeNames().GetNameToEnumMap().at(typ);
	
		// The new node is the destiation
		return MakeStandardAgent(ne);
	}
	else
	{
		return nullptr;
	}
}


