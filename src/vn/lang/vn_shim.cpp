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
	
	if( auto scope_block_gnomon = dynamic_pointer_cast<const ScopeBlockGnomon>(gnomon) )
		scope_block_gnomons.push_front( scope_block_gnomon ); // front is top
	else if( auto designation_gnomon = dynamic_pointer_cast<const DesignationGnomon>(gnomon) )
		designation_gnomons.insert( make_pair( designation_gnomon->name, designation_gnomon ) );
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
	shared_ptr<const DesignationGnomon> designation_gnomon;
	if( designation_gnomons.count(text) > 0 )	
	    designation_gnomon = designation_gnomons.at(text);
	    
	if( designation_gnomon )
		info.as_designated = designation_gnomon->pattern;
	else
		info.as_designated = nullptr;
		
	// Pick off the transformations we know about
	if( ascii && ToASCII(text)=="DeclarationOf" )
		return YY::VNLangParser::make_DECLARATION_OF(info, loc);
	else if( ascii && ToASCII(text)=="TypeOf" )
		return YY::VNLangParser::make_TYPE_OF(info, loc);
		
	// Determine the current scope from our weak gnomons
	const AvailableNodeData::ScopeBlock *current_scope_block = AvailableNodeData().GetRootBlock();
	for( weak_ptr<const ScopeBlockGnomon> wpg : scope_block_gnomons )
	{
		if( auto spg = wpg.lock() )
		{
			ASSERT( spg->scope_block );
			current_scope_block = spg->scope_block;
			break; // we only need the one at the front, because the names build up
		}
	}
	
	// See if we want to supply a block
	info.as_andata_block = nullptr;
	if( ascii && current_scope_block && current_scope_block->sub_blocks.count(ToASCII(text)) > 0 )
	{
		info.as_andata_block = current_scope_block->sub_blocks.at(ToASCII(text)).get();
		if( auto lb = dynamic_cast<const AvailableNodeData::LeafBlock *>(info.as_andata_block) )
		{
			if( AvailableNodeData().IsType(lb) )			
				return YY::VNLangParser::make_RESOLVED_TYPE(info, loc);
			else
				return YY::VNLangParser::make_RESOLVED_NONTYPE(info, loc);
		}
		else if( dynamic_cast<const AvailableNodeData::ScopeBlock *>(info.as_andata_block) )
			return YY::VNLangParser::make_SCOPE_NAME(info, loc);				
		else
			ASSERTFAIL("unreconised andata block");
	}
			
	if( designation_gnomon )
	{
		if( dynamic_cast<const NonTypeDesignationGnomon *>(designation_gnomon.get()) )
            return YY::VNLangParser::make_DESIGNATED_NONTYPE(info, loc);
        else if( dynamic_cast<const TypeDesignationGnomon *>(designation_gnomon.get()) )
            return YY::VNLangParser::make_DESIGNATED_TYPE(info, loc);
        else 
			ASSERTFAIL();
	}
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
	auto expired = [&](weak_ptr<const ScopeBlockGnomon> wpg)
	{
		return wpg.expired();
	};
	scope_block_gnomons.remove_if(expired);
}

