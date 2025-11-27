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

shared_ptr<Gnomon> VNShim::PushScopeRes( list<string> resolution )
{
	PurgeExpiredGnomons();
	auto spg = make_shared<Gnomon>(resolution);
	current_gnomons.push_front( spg ); // front is top
	return spg;
}



void VNShim::Designate( wstring name, TreePtr<Node> sub_pattern )
{
	designations.insert( make_pair(name, sub_pattern) );
}




YY::VNLangParser::symbol_type VNShim::OnUnquoted(string text, YY::VNLangParser::location_type loc) const
{
	return ProcessToken( ToUnicode(text), true, loc );
}


YY::VNLangParser::symbol_type VNShim::OnUnquoted(wstring text, YY::VNLangParser::location_type loc) const
{
	return ProcessToken( text, false, loc );
}


YY::VNLangParser::symbol_type VNShim::ProcessToken(wstring text, bool ascii, YY::VNLangParser::location_type loc) const
{
	(void)ascii;
	FTRACE(text)(" with ")(current_gnomons)("\n");

	// Where unicode is allowed, ascii is allowed too, so positive checks only
	YY::NameInfo info;
	if( designations.count(text) > 0 )	
		info.as_designated = designations.at(text);
	else
		info.as_designated = nullptr;
		
	for( weak_ptr<Gnomon> wpg : current_gnomons )
	{
		if( auto spg = wpg.lock() )
		{
			info.as_name_res_list = spg->resolution;
			break; // we only want one, because the names build up
		}
	}
			
	if( !info.as_name_res_list.empty() )
	{
		if( !ascii )
			throw YY::VNLangParser::syntax_error( loc, 
				"Resolved Unicode name not supported (resolved by " + Join(info.as_name_res_list, "::") + ")");		
		info.as_name_res_list.push_back(ToASCII(text));
		// TODO merge the locations
		FTRACE(	"Supply RESOLVED_NAME with: ")(info.as_name_res_list)("\n");
	}
	
	info.as_unicode = text;
	info.as_ascii = ToASCII(text);
	if( !info.as_name_res_list.empty() )
		return YY::VNLangParser::make_RESOLVED_NAME(info, loc);
	else if( info.as_designated )
         return YY::VNLangParser::make_NAMED_SUBTREE(info, loc);
    else if( ascii )
         return YY::VNLangParser::make_ASCII_NAME(info, loc);
    else
         return YY::VNLangParser::make_UNICODE_NAME(info, loc);
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


void VNShim::PurgeExpiredGnomons()
{
	current_gnomons.remove_if([](weak_ptr<Gnomon> wpg){return wpg.expired();});
}

// TODO NodeNames to tree form and walk immediately on sight of a name. Generate 
// errors early and replace list<string> with shared_ptr<NodeNameNode> etc.
// Don't forget the shortened identifier names eg Type, Instance etc

// Type hierarchy for gnomons: ScopeResGnomon : Gnomon for our scopes

// TODO parser should create the gnomon instances so 
// part_resolve	: resolved_name SCOPE_RES				{ $$ = shim->PushScopeRes($1); }
// becomes
// part_resolve	: resolved_name SCOPE_RES				{ $$ = ScopeResGnomon($1); shim->Push($$); }
// It's slightly longer but the subexpression shows (a) the gnomon is purely derived from $1,
// (b) we're pushing it to the parse stack and (c) finally offering it to the shim

// TODO designation action to call eg VNShim::PushDesignation() and shim keeps
// it under shared pointer, and returns it in the same form in info.as_designation

