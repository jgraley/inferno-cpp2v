#include "syntax.hpp"
#include "vn/lang/vn_lang.ypp.hpp"
#include "vn/lang/vn_lang.location.hpp"
#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

string Syntax::GetLoweredIdOrMacroName() const
{
	throw Unimplemented();
}


string Syntax::GetIdentifierName() const
{
	throw Unimplemented();
}


string Syntax::GetDesignationNameHint() const
{
	try { return GetLoweredIdOrMacroName(); }
	catch( Refusal & ) {}
	
	// If no token, just use the type
	string s = TYPE_ID_NAME(*this);
	s = GetInnermostTemplateParam(s);
	s = Traceable::Denamespace(s);
	
	// To lower case
	transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return tolower(c); });
	
	return "my_"+s; // feeble attempt to stop clashes with keywords in the VN language
}


bool Syntax::IsDesignationNamedIdentifier() const
{
	return false;
}


string Syntax::GetRender( VN::RendererInterface *, Production production, Policy )
{
	return GetRenderTerminal(production);
}


string Syntax::GetRenderTerminal( Production ) const
{
	throw Unimplemented();
}


Syntax::Production Syntax::GetMyProduction(const VN::RendererInterface *, Policy) const
{
	return GetMyProductionTerminal();
}


// What production do I become once rendered?
Syntax::Production Syntax::GetMyProductionTerminal() const
{
	throw Unimplemented();
}
    
    
// What production is expected as my operand in a declarator?
Syntax::Production Syntax::GetOperandInDeclaratorProduction() const
{
	throw Unimplemented();
} 


Syntax::Production Syntax::BoostPrecedence( Production prec )
{
	return (Production)((int)prec + 1);
}


int Syntax::GetPrecedence( Production prec )
{
	return (int)prec;
}
   

string::size_type Syntax::GetLineBreakThreshold()
{
	return 80;
}


string Syntax::GetKeyword(Policy) const
{	
	throw UnimplementedKeyword();
}


string Syntax::RenderNodeTypeName() const
{
	list<string> parts = Split( GetInnermostTemplateParam(TYPE_ID_NAME(*this)), "::" );
	
	if( parts.front()==DEFAULT_NODE_NAMESPACE )
		parts.pop_front();		
			
    return Join( parts, "::" );    
}


bool Syntax::IsType() const
{
	return false;
}


bool Syntax::IsSpecificIdentifier() const
{
	return false;
}


Syntax::Token Syntax::GetSignifierToken() const
{
	throw UnimplementedToken();
}


Syntax::Token Syntax::GetExplicitToken() const
{
	return YY::VNLangParser::token::TOK_EXPLICIT_NORMAL;
}


Syntax::Token Syntax::GetPrerestrictToken() const
{
	return YY::VNLangParser::token::TOK_PRERESTRICT_NORMAL;	
}


Syntax::Token Syntax::GetIdByNameToken() const
{
	return YY::VNLangParser::token::TOK_ID_BY_NAME_NORMAL;	
}


Syntax::Token Syntax::GetIdBuilderToken() const
{
	return YY::VNLangParser::token::TOK_ID_BUILDER_NORMAL;	
}


string Syntax::MyBestErrName() const try
{
	return DiagQuote( GetKeyword(Policy()) );
}		
catch( Unimplemented & )
{
	return DiagQuote(Traceable::TypeIdName( *this ));	
}


TreePtr<Node> Syntax::OnAccess( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect an access specifier (implement OnAccess()).");	
}


TreePtr<Node> Syntax::OnIdentifier( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect an identifier (implement OnIdentifier()).");	
}


TreePtr<Node> Syntax::OnPermission( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect const/mutable (implement OnPermission()).");	
}


TreePtr<Node> Syntax::OnDispatch( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect virtual (implement OnVirtual()).");
}


TreePtr<Node> Syntax::OnBases( list<TreePtr<Node>>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect base classes (implement OnBases()).");	
}


TreePtr<Node> Syntax::OnMembers( list<TreePtr<Node>>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect members (implement OnMembers()).");	
}


TreePtr<Node> Syntax::OnStatements( list<TreePtr<Node>>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect statements (implement OnStatements()).");	
}


TreePtr<Node> Syntax::OnType( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect a type (implement OnType()).");	
}


TreePtr<Node> Syntax::OnMemberInits( list<TreePtr<Node>>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect member initialisers (implement OnMemberInits()).");	
}


TreePtr<Node> Syntax::OnInitialiser( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect an initialiser (implement OnInitialiser()).");	
}


TreePtr<Node> Syntax::OnArgsList( list<TreePtr<Node>> args, Location loc )
{
	switch( args.size() )
	{
		case 0: // Ignore by default - there's no loss of information if no args were supplied 
		return TreePtrThis();	
		
		case 1:
		return OnSoloArg( SoloElementOf(args), loc );
		
		case 2:
		return OnTwoArgs( args.front(), args.back(), loc );
		
		default:
		throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
			MyBestErrName() + " does not expect multiple arguments (implement OnArgsList()).");	
	}
}


TreePtr<Node> Syntax::OnSoloArg( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect an argument (implement OnSoloArg()).");	
}


TreePtr<Node> Syntax::OnTwoArgs( TreePtr<Node>, TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect a pair of arguments (implement OnTwoArgs()).");	
}


TreePtr<Node> Syntax::OnArgumentation( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect an argumentation (implement OnArgumentation()).");		
}


TreePtr<Node> Syntax::OnObject( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect to be used on an object (implement OnObject()).");	
}

TreePtr<Node> Syntax::OnBody( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " does not expect a body (implement OnBody()).");		
} 


TreePtr<Node> Syntax::OnElseBody( TreePtr<Node>, Location loc )
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " cannot be used with an `else' clause (implement OnElseBody()).");		
} 


any Syntax::GetStartingScopeContext() const
{
	return any();	// std::any is nullable
}


void Syntax::UpdateContext( TreePtr<Node>, any &context, Location )
{
	ASSERT( !context.has_value() ); //	GetStartingScopeContext() was implemented therefore so should this be
	// No action otherwise because no context
}	


TreePtr<Node> Syntax::CreateDeclNode(bool, any &, Location loc) const
{
	throw YY::VNLangParser::syntax_error( any_cast<YY::VNLangParser::location_type>(loc),
		MyBestErrName() + " cannot act as a scope for declarations (implement CreateDeclNode()).");	
}
