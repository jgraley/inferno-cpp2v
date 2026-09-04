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


TreePtr<Node> ScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) 
{
	throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc),
			"Declarations not allowed inside " +
			GetMessageText() );
}


void ScopeGnomon::UpdateContext(any loc, TreePtr<Node> update_node)
{
	throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc),
			"Context update not allowed inside " +
			GetMessageText() + 
			" (update node is " + update_node->MyBestErrName() + ")");
}


RegularScopeGnomon::RegularScopeGnomon( TreePtr<Node> scope_node_ ) :
	scope_node(scope_node_) 
{
	ASSERT(scope_node);
}


string RegularScopeGnomon::GetMessageText() const 
{
	return scope_node->MyBestErrName() + " regular scope";
}


TreePtr<Node> RegularScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) 
{	
	any context( nullptr ); // TODO hold in class
	return scope_node->CreateDeclNode( static_keyword_specified, context, any_cast<YY::VNLangParser::location_type>(loc) );
}


TreePtr<Node> RegularScopeGnomon::GetNode() const
{
	return scope_node;
}


string ParameterisationScopeGnomon::GetMessageText() const 
{
	return "parameters scope";
}


TreePtr<Node> ParameterisationScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) 
{
	if( static_keyword_specified )
		throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(loc),
				"static is not allowed for parameters.");

	return MakeTreeNode<StandardAgentWrapper<CPPTree::Parameter>>();
}


AccessScopeGnomon::AccessScopeGnomon( TreePtr<Node> scope_node_ ) :
	RegularScopeGnomon(scope_node_)
{
	auto record = dynamic_pointer_cast<CPPTree::Record>(scope_node);
	ASSERT( record );
	// Get the type exactly right for std::any
	context = record->GetInitialContext();
}


TreePtr<Node> AccessScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) 
{
	return scope_node->CreateDeclNode( static_keyword_specified, context, any_cast<YY::VNLangParser::location_type>(loc) );
}


void AccessScopeGnomon::UpdateContext(any loc, TreePtr<Node> update_node)
{
	scope_node->UpdateContext( update_node, context, any_cast<YY::VNLangParser::location_type>(loc) );
}


UnknownScopeGnomon::UnknownScopeGnomon(string reason_) :
	reason( reason_ )
{
}


string UnknownScopeGnomon::GetMessageText() const 
{
	return "local scope";
}


TreePtr<Node> UnknownScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) 
{
	string note = 
		"\nNote: scope may be a surrounding code unit, compound, struct/class body,"
		"\nparams list, explicit scope node or pre-restriction to a declaration node type";	// TODO duplicated
	throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc),
			"Cannot disambiguate declaration under " + reason + "." + note );
}


PrerestrictScopeGnomon::PrerestrictScopeGnomon( TreePtr<Node> node_ ) :
	node(node_) 
{
}


string PrerestrictScopeGnomon::GetMessageText() const 
{
	return "prerestrict scope";
}


void PrerestrictScopeGnomon::UpdateContext(any, TreePtr<Node> )
{
	// There is no context so discard the update
}


TreePtr<Node> PrerestrictScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) 
{
	string note = 
		"\nNote: scope may be a surrounding code unit, compound, struct/class body,"
		"\nparams list, explicit scope node or pre-restriction to a declaration node type";	// TODO duplicated
	ASSERT( node );
	TreePtr<Node> instance = TreePtr<CPPTree::Instance>::DynamicCast(node);
	if( !instance )
		throw YY::VNLangParser::syntax_error(
					any_cast<YY::VNLangParser::location_type>(loc),
					"nearest prerestrict " + 
					DiagQuote(Traceable::TypeIdName( *(node) )) + 
					" cannot disambiguate an instance declaration" + 
					note); // TODO it could if the pre-restriction was to eg a Record etc			
	return instance;
}
