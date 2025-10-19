#include "syntax.hpp"
#include "common/common.hpp"

string Syntax::GetToken() const
{
	string s = TYPE_ID_NAME(*this);
	s = GetInnermostTemplateParam(s);
	s = Traceable::Denamespace(s);
	
	// To lower case
	std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return std::tolower(c); });
	
	return s;
}


string Syntax::GetRenderTerminal() const
{
	return ""; 
}


// What production do I become once rendered?
Syntax::Production Syntax::GetMyProduction() const
{
    return Production::UNDEFINED; 
}
    
// What production is expected as my operand in a declarator?
Syntax::Production Syntax::GetOperandInDeclaratorProduction() const
{
    return Production::UNDEFINED; 
} 


Syntax::Production Syntax::BoostPrecedence( Syntax::Production prec )
{
	return (Syntax::Production)((int)prec + 1);
}


int Syntax::GetPrecedence( Syntax::Production prec )
{
	return (int)prec;
}
    
