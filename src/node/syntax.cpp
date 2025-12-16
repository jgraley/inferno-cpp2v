#include "syntax.hpp"
#include "common/common.hpp"

#include "tree/cpptree.hpp"

const type_index Syntax::DefaultAccess = type_index(typeid(CPPTree::Public));


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


Syntax::Production Syntax::BoostPrecedence( Syntax::Production prec )
{
	return (Syntax::Production)((int)prec + 1);
}


int Syntax::GetPrecedence( Syntax::Production prec )
{
	return (int)prec;
}
   

string::size_type Syntax::GetLineBreakThreshold()
{
	return 80;
}
