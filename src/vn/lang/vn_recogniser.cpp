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
#include "vn/lang/vn_lang.ypp.hpp"
#include "vn/lang/vn_lang.lpp.hpp"
#include "vn/lang/vn_lang.location.hpp"
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

TreePtr<Node> VN::MakeStandardAgent(NodeTag ne)
{
	switch(ne)
	{
#define NODE(NS, NAME) \
	case NodeTag::NS##_##NAME: \
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
	return Recognise( ToUnicode(text), true, loc );
}


YY::VNLangParser::symbol_type VNLangRecogniser::OnUnquotedLexeme(wstring text, YY::VNLangParser::location_type loc) const
{
	return Recognise( text, false, loc );
}


YY::VNLangParser::symbol_type VNLangRecogniser::Recognise(wstring text, bool ascii, YY::VNLangParser::location_type loc) const
{
	// Where unicode is allowed, ascii is allowed too, so positive checks only
	YY::TokenMetadata metadata;
	metadata.as_unicode = text;
	metadata.as_ascii = ToASCII(text);
	metadata.as_andata_block = nullptr;
	metadata.node = nullptr;
	
	const ScopeGnomon *scope = nullptr;
	shared_ptr<const ScopeGnomon> spg = scope_gnomons.TryLockTop();
	if( spg && dynamic_cast<const NodeNameScopeGnomon *>(spg.get()) )
		return RecogniseInNodeNameScope(text, ascii, loc, metadata);
	else if( spg && dynamic_cast<const TransformNameScopeGnomon *>(spg.get()) )
		return RecogniseInTransformNameScope(text, ascii, loc, metadata);				
		
	try	{
		return RecogniseKeyword( text, ascii, metadata, loc );
	} catch( Unrecognised& ) {}	
		
	try	{
		return RecogniseDesignation( text, metadata, loc );
	} catch( Unrecognised& ) {}
		
	if( ascii )
         return YY::VNLangParser::make_ASCII_NAME(metadata, loc);
    else
         return YY::VNLangParser::make_UNICODE_NAME(metadata, loc);
}


YY::VNLangParser::symbol_type VNLangRecogniser::RecogniseInNodeNameScope(wstring text, bool ascii, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const
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
		const ANDBlock *sub_block = namespace_block->sub_blocks.at(ToASCII(text)).get();
		return CreateBlockToken( sub_block, loc, metadata );
	}
		
	if( default_namespace )
	{
		// Try the default namespace
		const ANDBlock *default_block = namespace_block->sub_blocks.at(DEFAULT_NODE_NAMESPACE).get();
		namespace_block = dynamic_cast<const AvailableNodeData::NamespaceBlock *>(default_block);
		ASSERT( namespace_block ); // Internal error: default block is not a namespace block
		if( namespace_block && namespace_block->sub_blocks.count(ToASCII(text)) > 0 )
		{
			const ANDBlock *sub_block = namespace_block->sub_blocks.at(ToASCII(text)).get();
			return CreateBlockToken( sub_block, loc, metadata );
		}
	}
			
	// In these scopes, there are no designations so we must succeed and can raise an error here if we don't
	throw YY::VNLangParser::syntax_error( loc,
	    SSPrintf("Unrecognised %s %s", DiagQuote(text).c_str(), GetContextText().c_str()) ); 
}


YY::VNLangParser::symbol_type VNLangRecogniser::CreateBlockToken(const ANDBlock *block, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const
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
	ASSERT(block->tag)("NodeBlock ")(*block)(" has no tag, so cannot create a node from it");
	metadata.node = MakeStandardAgent(block->tag.value());
	YY::VNLangParser::token::token_kind_type token_kind = metadata.node->GetResolvedToken();		
	return YY::VNLangParser::symbol_type( token_kind, std::move(metadata), std::move(loc) );		
}


YY::VNLangParser::symbol_type VNLangRecogniser::RecogniseInTransformNameScope(wstring text, bool ascii, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const
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


YY::VNLangParser::symbol_type VNLangRecogniser::RecogniseKeyword(wstring text, bool ascii, YY::TokenMetadata metadata, YY::VNLangParser::location_type loc) const
{
	if( !ascii ) // Keywords are only ASCII
		throw Unrecognised();

	string ascii_text = ToASCII(text);

	optional<NodeTag> tag = AvailableNodeData().TryGetByKeywordIfToken( ascii_text );
	if( !tag )
		throw Unrecognised();
	metadata.node = MakeStandardAgent(tag.value());
	return YY::VNLangParser::symbol_type( metadata.node->GetToken(), std::move(metadata), std::move(loc) );
}


YY::VNLangParser::symbol_type VNLangRecogniser::RecogniseDesignation(wstring text, YY::TokenMetadata metadata, YY::VNLangParser::location_type loc) const
{
	shared_ptr<const DesignationGnomon> designation_gnomon;
	if( designation_gnomons.count(text) > 0 )	
	    designation_gnomon = designation_gnomons.at(text);
	else
		throw Unrecognised();
	
	metadata.node = designation_gnomon->node;
	return YY::VNLangParser::symbol_type( designation_gnomon->token, std::move(metadata), std::move(loc) );
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


