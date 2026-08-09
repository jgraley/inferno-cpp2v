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
	ASSERT( gnomon );
	
	if( auto scope_gnomon = dynamic_pointer_cast<const ScopeGnomon>(gnomon) )
		scope_gnomons.Push( scope_gnomon ); // front is top
	else if( auto resolver_gnomon = dynamic_pointer_cast<const ResolverGnomon>(gnomon) )
		resolver_gnomons.Push( resolver_gnomon ); // front is top
	else if( auto designation_gnomon = dynamic_pointer_cast<const DesignationGnomon>(gnomon) )
		designation_gnomons.insert( make_pair( designation_gnomon->name, designation_gnomon ) );
	else 
		ASSERT(false)("Recogniser doesn't know about gnomon: ")(*gnomon);
}


YY::VNLangParser::symbol_type VNLangRecogniser::OnUnquotedLexeme(string text, YY::VNLangParser::location_type loc) const
{
	return ProcessLexeme( ToUnicode(text), true, loc );
}


YY::VNLangParser::symbol_type VNLangRecogniser::OnUnquotedLexeme(wstring text, YY::VNLangParser::location_type loc) const
{
	return ProcessLexeme( text, false, loc );
}


YY::VNLangParser::symbol_type VNLangRecogniser::RecogniseKeyword(string text, const YY::TokenMetadata &metadata, YY::VNLangParser::location_type loc) const
{
	if(  text=="this" )
		return YY::VNLangParser::make_KEYWORD_PRIMARY_OP(metadata, loc);
	else if( text=="break" ||
			 text=="continue" ) 
		return YY::VNLangParser::make_KEYWORD_SIMPLE_STMT(metadata, loc);
	else if( text=="return" ||
	         text=="goto" ) 
		return YY::VNLangParser::make_KEYWORD_SPACE_SEP_STMT(metadata, loc);
	else if( text=="switch" ||
	         text=="for" ) 
		return YY::VNLangParser::make_KEYWORD_ARGS_BODY_STMT(metadata, loc);
	else if( text=="if" )
		return YY::VNLangParser::make_KEYWORD_ARGS_BODY_CHAIN_STMT(metadata, loc);
	else if( text=="else" )
		return YY::VNLangParser::make_CHAINING_KEYWORD(metadata, loc);
	else if( text=="while" )
		return YY::VNLangParser::make_WHILE_KEYWORD(metadata, loc);
	else if( text=="do" )
		return YY::VNLangParser::make_DO_KEYWORD(metadata, loc);
	else if( text=="case" )
		return YY::VNLangParser::make_CASE_KEYWORD(metadata, loc);
	else if( text=="default" )
		return YY::VNLangParser::make_DEFAULT_KEYWORD(metadata, loc);
	else if( text=="true" ||
			 text=="false" )
		return YY::VNLangParser::make_BOOL_LITERAL(text=="true", loc);
	else if( text=="typename" )
		return YY::VNLangParser::make_TYPENAME(text, loc);
	else if( text=="char" ||
	         text=="bool" ||
	         text=="short" ||
	         text=="int" ||
	         text=="long" ||
	         text=="signed" ||
	         text=="unsigned" ||
	         text=="float" ||
	         text=="double" ||
	         text=="void" )
		return YY::VNLangParser::make_TYPE_SPECIFIER(text, loc);
	else if( text=="sizeof" ||
	         text=="alignof" ) 
		return YY::VNLangParser::make_FUNC_ON_TYPE(metadata, loc);
	else if( text=="class" ||
	         text=="struct" ||
	         text=="union" )
		return YY::VNLangParser::make_CLASS_KEYWORD(metadata, loc);
	else if( text=="enum" ) 
		return YY::VNLangParser::make_ENUM_KEYWORD(metadata, loc);
	else if( text=="public" ||
	         text=="private" ||
	         text=="protected" ||
	         text=="const" ||
	         text=="mutable" ||
	         text=="virtual" )
		return YY::VNLangParser::make_QUAL_NODE_KEYWORD(metadata, loc);
	else if( text=="static" )
		return YY::VNLangParser::make_STATIC_KEYWORD(metadata, loc);
	else if( text=="typedef" )
		return YY::VNLangParser::make_TYPEDEF_KEYWORD(metadata, loc);
	else 
		throw Unrecognised();
}

YY::VNLangParser::symbol_type VNLangRecogniser::ProcessLexeme(wstring text, bool ascii, YY::VNLangParser::location_type loc) const
{
	// Where unicode is allowed, ascii is allowed too, so positive checks only
	YY::TokenMetadata metadata;
	metadata.as_unicode = text;
	metadata.as_ascii = ToASCII(text);
	metadata.as_andata_block = nullptr;
	
	const ScopeGnomon *scope = nullptr;
	shared_ptr<const ScopeGnomon> spg = scope_gnomons.TryLockTop();
	if( spg && dynamic_cast<const NodeNameScopeGnomon *>(spg.get()) )
		return ProcessLexemeInNodeNameScope(text, ascii, loc, metadata);
	else if( spg && dynamic_cast<const TransformNameScopeGnomon *>(spg.get()) )
		return ProcessLexemeInTransformNameScope(text, ascii, loc, metadata);		
	
	shared_ptr<const DesignationGnomon> designation_gnomon;
	if( designation_gnomons.count(text) > 0 )	
	    designation_gnomon = designation_gnomons.at(text);
	    
	if( designation_gnomon )
		metadata.as_designated = designation_gnomon->pattern;
	else
		metadata.as_designated = nullptr;
		
	try
	{
		if( ascii ) // Keywords are only ASCII
			return RecogniseKeyword( ToASCII(text), metadata, loc );
	}
	catch( Unrecognised& )
	{
		// Just carry on
	}
		
	if( designation_gnomon )
	{
		if( dynamic_cast<const NormalDesignationGnomon *>(designation_gnomon.get()) )
            return YY::VNLangParser::make_DESIGNATED_NORMAL(metadata, loc);
        else if( dynamic_cast<const TypeDesignationGnomon *>(designation_gnomon.get()) )
            return YY::VNLangParser::make_DESIGNATED_TYPE(metadata, loc);
        else if( dynamic_cast<const DeclarationDesignationGnomon *>(designation_gnomon.get()) )
            return YY::VNLangParser::make_DESIGNATED_DECL(metadata, loc);
        else if( dynamic_cast<const QualifierDesignationGnomon *>(designation_gnomon.get()) )
            return YY::VNLangParser::make_DESIGNATED_QUAL(metadata, loc);
        else if( dynamic_cast<const CompoundDesignationGnomon *>(designation_gnomon.get()) )
            return YY::VNLangParser::make_DESIGNATED_COMPOUND(metadata, loc);
		else 
			ASSERTFAIL();
	}
    else if( ascii )
         return YY::VNLangParser::make_ASCII_NAME(metadata, loc);
    else
         return YY::VNLangParser::make_UNICODE_NAME(metadata, loc);
}


YY::VNLangParser::symbol_type VNLangRecogniser::ProcessLexemeInNodeNameScope(wstring text, bool ascii, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const
{
	if( !ascii )	
		throw YY::VNLangParser::syntax_error( loc,
	        SSPrintf("Unrecognised non-ASCII %s %s", DiagQuote(text).c_str(), GetContextText().c_str()) );;
		
	// Determine the current scope from our weak gnomons
	const AvailableNodeData::NamespaceBlock *namespace_block = AvailableNodeData().GetNodeNamesRoot();
	bool default_namespace = true;
	shared_ptr<const ResolverGnomon> spg = resolver_gnomons.TryLockTop();
	if( spg )
	{
		ASSERT( spg->namespace_block );
		namespace_block = spg->namespace_block;
		default_namespace = false;
	}

	// See if we want to supply a block
	if( namespace_block && namespace_block->sub_blocks.count(ToASCII(text)) > 0 )
	{
		const AvailableNodeData::Block *sub_block = namespace_block->sub_blocks.at(ToASCII(text)).get();
		return CreateBlockToken( sub_block, loc, metadata );
	}
		
	if( default_namespace )
	{
		// Try the default namespace
		const AvailableNodeData::Block *default_block = namespace_block->sub_blocks.at(DEFAULT_NODE_NAMESPACE).get();
		namespace_block = dynamic_cast<const AvailableNodeData::NamespaceBlock *>(default_block);
		ASSERT( namespace_block ); // Internal error: default block is not a namespace block
		if( namespace_block && namespace_block->sub_blocks.count(ToASCII(text)) > 0 )
		{
			const AvailableNodeData::Block *sub_block = namespace_block->sub_blocks.at(ToASCII(text)).get();
			return CreateBlockToken( sub_block, loc, metadata );
		}
	}
			
	// In these scopes, there are no designations so we must succeed and can raise an error here if we don't
	throw YY::VNLangParser::syntax_error( loc,
	    SSPrintf("Unrecognised %s %s", DiagQuote(text).c_str(), GetContextText().c_str()) ); 
}


YY::VNLangParser::symbol_type VNLangRecogniser::CreateBlockToken(const AvailableNodeData::Block *block, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const
{
	metadata.as_andata_block = block; // return it to the parser whatever it is
	if( auto lb = dynamic_cast<const AvailableNodeData::NodeBlock *>(block) )
		return CreateNodeToken(lb, loc, metadata);
	else if( dynamic_cast<const AvailableNodeData::NamespaceBlock *>(block) )
		return YY::VNLangParser::make_NODE_NAMESPACE(metadata, loc);				
	else
		ASSERTFAIL("bad andata block");
}


YY::VNLangParser::symbol_type VNLangRecogniser::CreateNodeToken(const AvailableNodeData::NodeBlock *block, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const
{
	if( AvailableNodeData().IsQualifier(block) )			
		return YY::VNLangParser::make_RESOLVED_QUAL(metadata, loc);
	if( AvailableNodeData().IsMemberInit(block) )			
		return YY::VNLangParser::make_RESOLVED_MEMB_INIT(metadata, loc);
	else if( AvailableNodeData().IsDeclaration(block) )			
		return YY::VNLangParser::make_RESOLVED_DECL(metadata, loc);
	else if( AvailableNodeData().IsType(block) )			
		return YY::VNLangParser::make_RESOLVED_TYPE(metadata, loc);
	else
		return YY::VNLangParser::make_RESOLVED_NORMAL(metadata, loc);
}


YY::VNLangParser::symbol_type VNLangRecogniser::ProcessLexemeInTransformNameScope(wstring text, bool ascii, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const
{
	// Transformations that act on normal scopes (instances, in this case)
	if( ascii && ToASCII(text)=="TypeOf" )
		return YY::VNLangParser::make_TRANSFORM_NAME_NORMAL(metadata, loc);					

	// Transformations that act on unified scopes (instances or types, in this case)
	if( ascii && ToASCII(text)=="DeclarationOf" )
		return YY::VNLangParser::make_TRANSFORM_NAME_NORMAL(metadata, loc);

	// Transformations that act on unified scopes (instances or types, in this case)
	if( ascii && ToASCII(text)=="TypeDeclarationOf" )
		return YY::VNLangParser::make_TRANSFORM_NAME_TYPE(metadata, loc);

	// In these scopes, there are no designations so we must succeed and can raise an error here if we don#t
	throw YY::VNLangParser::syntax_error( loc,
	    SSPrintf("Unrecognised: %s %s", DiagQuote(text).c_str(), GetContextText().c_str()) ); 
}


string VNLangRecogniser::GetContextText() const
{
	list<string> ls;
	scope_gnomons.For( [&](const shared_ptr<const ScopeGnomon> &spg)
	{
		ls.push_back("inside "+spg->GetMessageText());
	} );
	
	return Join( ls, ", " );
}


