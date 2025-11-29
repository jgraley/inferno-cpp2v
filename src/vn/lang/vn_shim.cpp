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
// YY::VNLangParser::symbol_type::symbol_type( int tok, VNLangRecogniser::NameData, location_type l )   <--- a constructor
// with tok = token::TOK_SOME_KIND_OF_NAME
// and VNLangRecogniser::NameData being the type of TOK_SOME_KIND_OF_NAME etc as specified in the .ypp file
//
// Scanner rule is
// xyz   { return recogniser.OnUnicodeName( wstr(), location() ); }
//
// defined like YY::VNLangParser::symbol_type VNLangRecogniser::OnUnicodeName( wstring v, location_type l );   <--- or "any" for location
//
// ALL strings found by parser go in here, ASCII or Unicode, quoted or not, including keywords.
// There's a separate entry point for each parsing token. EP for quoted should unquote.
// All call a common analysis function:
// - designations
// - Node type names
// - Indeitifer subtypes
// Returning this data in a struct VNLangRecogniser::NameData which will also be passed to parser
// Then do a priority-based analysis that leads to a choice of parser token. Presumably, we
// can go top priotity first, doing eg
// if (cond)
//     return YY::VNLangParser::make_SOME_KIND_OF_NAME( string, name_data );
//
// Stored designations: store the PARSED "kind" and recover in recogniser. Thus, we only need the kind 
// for explicit nodes, and we don't need to go hunting through the pattern. Mis-parses like 
// conjunction( type, not-type ) will cause parse errors.

void VNLangRecogniser::AddGnomon( shared_ptr<Gnomon> gnomon )
{
	PurgeExpiredGnomons();
	
	if( auto scope_res_gnomon = dynamic_pointer_cast<const ANDataBlockGnomon>(gnomon) )
		scope_res_gnomons.push_front( scope_res_gnomon ); // front is top
}


void VNLangRecogniser::Designate( wstring name, TreePtr<Node> sub_pattern )
{
	designations.insert( make_pair(name, sub_pattern) );
}


YY::VNLangParser::symbol_type VNLangRecogniser::OnUnquoted(string text, YY::VNLangParser::location_type loc) const
{
	return ProcessToken( ToUnicode(text), true, loc );
}


YY::VNLangParser::symbol_type VNLangRecogniser::OnUnquoted(wstring text, YY::VNLangParser::location_type loc) const
{
	return ProcessToken( text, false, loc );
}


YY::VNLangParser::symbol_type VNLangRecogniser::ProcessToken(wstring text, bool ascii, YY::VNLangParser::location_type loc) const
{
	(void)ascii;

	// Where unicode is allowed, ascii is allowed too, so positive checks only
	YY::NameInfo info;
	info.as_unicode = text;
	info.as_ascii = ToASCII(text);
	if( designations.count(text) > 0 )	
		info.as_designated = designations.at(text);
	else
		info.as_designated = nullptr;
		
	const AvailableNodeData::Block *current_block = AvailableNodeData().GetRootBlock();
		
	for( weak_ptr<const ANDataBlockGnomon> wpg : scope_res_gnomons )
	{
		if( auto spg = wpg.lock() )
		{
			if( spg->andata_block )
				current_block = spg->andata_block;
			break; // we only need the one at the front, because the names build up
		}
	}

	info.as_andata_block = nullptr;
	if( ascii )
	{
		FTRACE("ASCII token ")(ToASCII(text))(" current_block:\n")(*current_block)("\n");
		if( auto scope_block = dynamic_cast<const AvailableNodeData::ScopeBlock *>(current_block) )
		{
			if( scope_block->sub_blocks.count(ToASCII(text)) > 0 )
			{
				info.as_andata_block = scope_block->sub_blocks.at(ToASCII(text)).get();
				if( auto node_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(info.as_andata_block) )
				{
					FTRACE(	"Supply RESOLVED_NAME with: ")(info.as_andata_block)("\n");
					return YY::VNLangParser::make_RESOLVED_NAME(info, loc);
				}
			}
		}
	}
			
	FTRACE(	"Supply something else\n");
	if( info.as_designated )
         return YY::VNLangParser::make_NAMED_SUBTREE(info, loc);
    else if( ascii )
         return YY::VNLangParser::make_ASCII_NAME(info, loc);
    else
         return YY::VNLangParser::make_UNICODE_NAME(info, loc);
}


TreePtr<Node> VNLangRecogniser::TryGetArchetype( list<string> typ ) const
{
	if( AvailableNodeData().GetNameToEnumMap().count(typ) > 0 )
	{		
		NodeEnum ne = AvailableNodeData().GetNameToEnumMap().at(typ);
	
		// The new node is the destiation
		return MakeStandardAgent(ne);
	}
	else
	{
		return nullptr;
	}
}


void VNLangRecogniser::PurgeExpiredGnomons()
{
	auto expired = [&](weak_ptr<const ANDataBlockGnomon> wpg)
	{
		return wpg.expired();
	};
	scope_res_gnomons.remove_if(expired);
}


// TODO designation action to push a new DesignationGnomon and recogniser keeps
// it under shared pointer, and returns it in the same form in the info.

