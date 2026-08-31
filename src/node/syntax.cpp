#include "syntax.hpp"
#include "common/common.hpp"

#include "tree/cpptree.hpp"

string Syntax::GetLoweredIdName() const
{
	throw Unimplemented();
}


string Syntax::GetIdentifierName() const
{
	throw Unimplemented();
}


string Syntax::GetDesignationNameHint() const
{
	try { return GetLoweredIdName(); }
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


YY::VNLangParser::token::token_kind_type Syntax::GetKeywordToken() const
{
	throw UnimplementedToken();
}


YY::VNLangParser::token::token_kind_type Syntax::GetCompleteToken() const
{
	return YY::VNLangParser::token::TOK_RESOLVED_NORMAL;
}


string Syntax::MyBestErrName() const try
{
	return DiagQuote( GetKeyword(Policy()) );
}		
catch( Unimplemented & )
{
	return DiagQuote(Traceable::TypeIdName( *this ));	
}


TreePtr<Node> Syntax::OnIdentifier( TreePtr<Node>, YY::VNLangParser::location_type loc )
{
	throw YY::VNLangParser::syntax_error( loc,
		MyBestErrName() + " does not expect an identifier.");	
}


TreePtr<Node> Syntax::OnBases( list<TreePtr<Node>>, YY::VNLangParser::location_type loc )
{
	throw YY::VNLangParser::syntax_error( loc,
		MyBestErrName() + " does not expect base classes.");	
}


TreePtr<Node> Syntax::OnMembers( list<TreePtr<Node>>, YY::VNLangParser::location_type loc )
{
	throw YY::VNLangParser::syntax_error( loc,
		MyBestErrName() + " does not expect members.");	
}


TreePtr<Node> Syntax::OnType( TreePtr<Node>, YY::VNLangParser::location_type loc )
{
	throw YY::VNLangParser::syntax_error( loc,
		MyBestErrName() + " does not expect a type.");	
}


TreePtr<Node> Syntax::OnArgsList( list<TreePtr<Node>>, YY::VNLangParser::location_type loc )
{
	throw YY::VNLangParser::syntax_error( loc,
		MyBestErrName() + " does not expect arguments.");	
}


TreePtr<Node> Syntax::OnBody( TreePtr<Node>, YY::VNLangParser::location_type loc )
{
	throw YY::VNLangParser::syntax_error( loc,
		MyBestErrName() + " does not expect a body.");		
} 


TreePtr<Node> Syntax::OnElseBody( TreePtr<Node>, YY::VNLangParser::location_type loc )
{
	throw YY::VNLangParser::syntax_error( loc,
		MyBestErrName() + " cannot be used with an `else' clause.");		
} 
