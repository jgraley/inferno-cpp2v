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


TreePtr<Node> ScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) const 
{
	throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc),
			"Declarations not allowed inside " +
			GetMessageText() );
}


RegularScopeGnomon::RegularScopeGnomon( TreePtr<Node> node_ ) :
	node(node_) 
{
}


string RegularScopeGnomon::GetMessageText() const 
{
	return node->MyBestErrName() + " scope";
}


TreePtr<Node> RegularScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) const 
{
	return node->CreateDeclNode( static_keyword_specified, any_cast<YY::VNLangParser::location_type>(loc) );
}


string ParameterisationScopeGnomon::GetMessageText() const 
{
	return "parameters scope";
}


TreePtr<Node> ParameterisationScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) const 
{
	if( static_keyword_specified )
		throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(loc),
				"static is not allowed for parameters.");

	return MakeTreeNode<StandardAgentWrapper<CPPTree::Parameter>>();
}


RecordScopeGnomon::RecordScopeGnomon( TreePtr<Node> initial_access, TreePtr<Node> record_type_ ) :
	current_access( initial_access ),
	record_type( record_type_ ) 
{
}


string RecordScopeGnomon::GetMessageText() const 
{
	return "record scope";
}


TreePtr<Node> RecordScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) const 
{
	if( static_keyword_specified )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::Global>>(); 
	return MakeTreeNode<StandardAgentWrapper<CPPTree::Member>>();
}


UnknownScopeGnomon::UnknownScopeGnomon(string reason_) :
	reason( reason_ )
{
}


string UnknownScopeGnomon::GetMessageText() const 
{
	return "local scope";
}


TreePtr<Node> UnknownScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) const 
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


TreePtr<Node> PrerestrictScopeGnomon::GetDeclarationNode(any loc, bool static_keyword_specified) const 
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
