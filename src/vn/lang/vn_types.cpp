#include "vn_types.hpp"

// Watch the deps here: parser header file includes this
#include "node/tree_ptr.hpp"
#include "tree/cpptree.hpp"
#include "tree/localtree.hpp"
#include "vn/lang/vn_lang.ypp.hpp"
#include "vn/lang/vn_lang.lpp.hpp"
#include "vn/lang/vn_lang.location.hpp"
#include "vn/agents/standard_agent.hpp"

#include <any> // to dep-break the generated headers




using namespace VN;
			
Gnomon::~Gnomon()
{ 
}


TreePtr<Node> ScopeGnomon::GetDeclarationNode(Syntax::Location loc) 
{
	throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc),
			"Declarations not allowed inside " +
			GetMessageText() );
}


void ScopeGnomon::UpdateContext(Syntax::Location loc, TreePtr<Node> update_node)
{
	// Allow this to slide. Outside of contextual scopes we can still have normal
	// path to access, which only allows one closely bound declaration. This
	// decl will be updated via OnAccess()
}


ContextScopeGnomon::ContextScopeGnomon(any context_) :
	context(context_)
{
}


string ContextScopeGnomon::GetMessageText() const 
{
	return "context scope";
}


void ContextScopeGnomon::UpdateContext(Syntax::Location loc, TreePtr<Node> update_node)
{
	context = TreePtr<CPPTree::AccessSpec>::DynamicCast(update_node);
}


RegularScopeGnomon::RegularScopeGnomon( TreePtr<Node> scope_node_ ) :
	ContextScopeGnomon( scope_node_->GetStartingScopeContext() ),
	scope_node( scope_node_ )
{
	ASSERT(scope_node);
}


string RegularScopeGnomon::GetMessageText() const 
{
	return scope_node->MyBestErrName() + " regular scope";
}


TreePtr<Node> RegularScopeGnomon::GetDeclarationNode(Syntax::Location loc) 
{	
	return scope_node->CreateDeclNode( any_cast<YY::VNLangParser::location_type>(loc) );
}


void RegularScopeGnomon::UpdateContext(Syntax::Location loc, TreePtr<Node> update_node)
{
	scope_node->UpdateContext( update_node, context, any_cast<YY::VNLangParser::location_type>(loc) );
}


TreePtr<Node> RegularScopeGnomon::GetNode() const
{
	return scope_node;
}


string ParameterisationScopeGnomon::GetMessageText() const 
{
	return "parameters scope";
}


TreePtr<Node> ParameterisationScopeGnomon::GetDeclarationNode(Syntax::Location loc) 
{
	return MakeTreeNode<CPPTree::Parameter>();
}


UnknownScopeGnomon::UnknownScopeGnomon(string reason_) :
	reason( reason_ )
{
}


string UnknownScopeGnomon::GetMessageText() const 
{
	return "unknown scope";
}


TreePtr<Node> UnknownScopeGnomon::GetDeclarationNode(Syntax::Location loc) 
{
	string note = 
		"\nNote: scope may be a surrounding code unit, compound, struct/class body,"
		"\nparams list, explicit scope node or pre-restriction to a declaration node type";	// TODO duplicated
	throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc),
			"Cannot disambiguate declaration under " + reason + "." + note );
}
