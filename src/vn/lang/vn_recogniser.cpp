#include "vn_recogniser.hpp"

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
#include "vn_actions.hpp"

#include <iostream>
#include <fstream>
#include <cctype>

using namespace CPPTree; // TODO should not need
using namespace VN;
using namespace reflex;

void VNLangRecogniser::AddGnomon( shared_ptr<Gnomon> gnomon )
{
	PurgeExpiredGnomons();
	ASSERT( gnomon );
	
	if( auto scope_gnomon = dynamic_pointer_cast<const ScopeGnomon>(gnomon) )
		scope_gnomons.push_front( scope_gnomon ); // front is top
	else if( auto resolver_gnomon = dynamic_pointer_cast<const ResolverGnomon>(gnomon) )
		resolver_gnomons.push_front( resolver_gnomon ); // front is top
	else if( auto designation_gnomon = dynamic_pointer_cast<const DesignationGnomon>(gnomon) )
		designation_gnomons.insert( make_pair( designation_gnomon->name, designation_gnomon ) );
	else 
		ASSERT(false)("Recogniser doesn't know about gnomon: ")(*gnomon);
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
	// Where unicode is allowed, ascii is allowed too, so positive checks only
	YY::TokenMetadata metadata;
	metadata.as_unicode = text;
	metadata.as_ascii = ToASCII(text);
	metadata.as_andata_block = nullptr;
	
	const ScopeGnomon *scope = nullptr;
	for( weak_ptr<const ScopeGnomon> wpg : scope_gnomons ) // loops from top down
	{
		if( auto spg = wpg.lock() )
		{
			if( dynamic_cast<const NodeNameScopeGnomon *>(spg.get()) )
				return ProcessTokenInNodeNameScope(text, ascii, loc, metadata);
			else if( dynamic_cast<const TransformNameScopeGnomon *>(spg.get()) )
				return ProcessTokenTransformNameScope(text, ascii, loc, metadata);
			
			break; // Only considering innermost for now			
		}
	}	
	
	shared_ptr<const DesignationGnomon> designation_gnomon;
	if( designation_gnomons.count(text) > 0 )	
	    designation_gnomon = designation_gnomons.at(text);
	    
	if( designation_gnomon )
		metadata.as_designated = designation_gnomon->pattern;
	else
		metadata.as_designated = nullptr;
		
	// Pick off keywords
	if( ascii && ToASCII(text)=="this" )
		return YY::VNLangParser::make_NORM_TERM_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="break" )
		return YY::VNLangParser::make_PRIMITIVE_STMT_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="continue" )
		return YY::VNLangParser::make_PRIMITIVE_STMT_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="return" )
		return YY::VNLangParser::make_SPACE_SEP_STMT_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="goto" )
		return YY::VNLangParser::make_SPACE_SEP_STMT_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="switch" )
		return YY::VNLangParser::make_ARGS_BODY_STMT_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="for" )
		return YY::VNLangParser::make_ARGS_BODY_STMT_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="if" )
		return YY::VNLangParser::make_ARGS_BODY_CHAIN_STMT_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="else" )
		return YY::VNLangParser::make_PRIMITIVE_CHAIN_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="while" )
		return YY::VNLangParser::make_WHILE_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="do" )
		return YY::VNLangParser::make_DO_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="case" )
		return YY::VNLangParser::make_CASE_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="default" )
		return YY::VNLangParser::make_DEFAULT_KEYWORD(metadata, loc);
	else if( ascii && ToASCII(text)=="true" )
		return YY::VNLangParser::make_BOOL_LITERAL(true, loc);
	else if( ascii && ToASCII(text)=="false" )
		return YY::VNLangParser::make_BOOL_LITERAL(false, loc);
	else if( ascii && ToASCII(text)=="typename" )
		return YY::VNLangParser::make_TYPENAME(ToASCII(text), loc);
	else if( ascii && 
	         ( ToASCII(text)=="char" ||
	           ToASCII(text)=="bool" ||
	           ToASCII(text)=="short" ||
	           ToASCII(text)=="int" ||
	           ToASCII(text)=="long" ||
	           ToASCII(text)=="signed" ||
	           ToASCII(text)=="unsigned" ||
	           ToASCII(text)=="float" ||
	           ToASCII(text)=="double" ||
	           ToASCII(text)=="void" ) )	           
		return YY::VNLangParser::make_TYPE_SPECIFIER(ToASCII(text), loc);
		
	if( designation_gnomon )
	{
		if( dynamic_cast<const NonTypeDesignationGnomon *>(designation_gnomon.get()) )
            return YY::VNLangParser::make_DESIGNATED_NORMAL(metadata, loc);
        else if( dynamic_cast<const TypeDesignationGnomon *>(designation_gnomon.get()) )
            return YY::VNLangParser::make_DESIGNATED_TYPE(metadata, loc);
        else 
			ASSERTFAIL();
	}
    else if( ascii )
         return YY::VNLangParser::make_ASCII_NAME(metadata, loc);
    else
         return YY::VNLangParser::make_UNICODE_NAME(metadata, loc);
}


YY::VNLangParser::symbol_type VNLangRecogniser::ProcessTokenInNodeNameScope(wstring text, bool ascii, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const
{
	if( !ascii )	
		return;
		
	// Determine the current scope from our weak gnomons
	const AvailableNodeData::NamespaceBlock *namespace_block = AvailableNodeData().GetNodeNamesRoot();
	bool default_namespace = true;
	for( weak_ptr<const ResolverGnomon> wpg : resolver_gnomons )
	{
		if( auto spg = wpg.lock() )
		{
			ASSERT( spg->namespace_block );
			namespace_block = spg->namespace_block;
			default_namespace = false;
			break; // we only need the one at the front, because the names build up
		}
	}
		
	// See if we want to supply a block
	if( namespace_block && namespace_block->sub_blocks.count(ToASCII(text)) > 0 )
	{
		const AvailableNodeData::Block *sub_block = namespace_block->sub_blocks.at(ToASCII(text)).get();
		metadata.as_andata_block = sub_block; // return it to the parser whatever it is
		if( auto lb = dynamic_cast<const AvailableNodeData::LeafBlock *>(sub_block) )
		{
			if( AvailableNodeData().IsType(lb) )			
				return YY::VNLangParser::make_RESOLVED_TYPE(metadata, loc);
			else
				return YY::VNLangParser::make_RESOLVED_NORMAL(metadata, loc);
		}
		else if( dynamic_cast<const AvailableNodeData::NamespaceBlock *>(sub_block) )
			return YY::VNLangParser::make_NODE_NAMESPACE(metadata, loc);				
		else
			ASSERTFAIL("bad andata block");
	}
		
	if( default_namespace )
	{
		// Try the default
		namespace_block = namespace_block->sub_blocks.at(DEFAULT_NODE_NAMESPACE).get();
		if( namespace_block && namespace_block->sub_blocks.count(ToASCII(text)) > 0 )
		{
			const AvailableNodeData::Block *sub_block = namespace_block->sub_blocks.at(ToASCII(text)).get();
			metadata.as_andata_block = sub_block; // return it to the parser whatever it is
			if( auto lb = dynamic_cast<const AvailableNodeData::LeafBlock *>(sub_block) )
			{
				if( AvailableNodeData().IsType(lb) )			
					return YY::VNLangParser::make_RESOLVED_TYPE(metadata, loc);
				else
					return YY::VNLangParser::make_RESOLVED_NORMAL(metadata, loc);
			}
			else if( dynamic_cast<const AvailableNodeData::NamespaceBlock *>(sub_block) )
				return YY::VNLangParser::make_NODE_NAMESPACE(metadata, loc);				
			else
				ASSERTFAIL("bad andata block");			
		}
	}
			
	// In these scopes, there are no designations so we must succeed and can raise an error here if we don't
	throw YY::VNLangParser::syntax_error( loc,
	    SSPrintf("Unrecognised %s %s", DiagQuote(text).c_str(), GetContextText().c_str()) ); 
}


YY::VNLangParser::symbol_type VNLangRecogniser::ProcessTokenTransformNameScope(wstring text, bool ascii, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const
{
	// Transformations that act on normal scopes (instances, in this case)
	if( ascii && ToASCII(text)=="TypeOf" )
		return YY::VNLangParser::make_TRANSFORM_NAME_NORMAL(metadata, loc);					

	// Transformations that act on unified scopes (instances or types, in this case)
	if( ascii && ToASCII(text)=="DeclarationOf" )
		return YY::VNLangParser::make_TRANSFORM_NAME_UNIFIED(metadata, loc);

	// In these scopes, there are no designations so we must succeed and can raise an error here if we don#t
	throw YY::VNLangParser::syntax_error( loc,
	    SSPrintf("Unrecognised: %s %s", DiagQuote(text).c_str(), GetContextText().c_str()) ); 
}


void VNLangRecogniser::PurgeExpiredGnomons()
{
	auto expired = [&](weak_ptr<const Gnomon> wpg)
	{
		return wpg.expired();
	};
	resolver_gnomons.remove_if(expired);
	scope_gnomons.remove_if(expired);
}


string VNLangRecogniser::GetContextText() const
{
	list<string> ls;
	for( weak_ptr<const ScopeGnomon> wpg : scope_gnomons )
	{
		if( auto spg = wpg.lock() )
			ls.push_back("inside "+spg->GetMessageText());
	}
	return Join( ls, ", " );
}


