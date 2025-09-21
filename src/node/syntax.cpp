#include "syntax.hpp"
#include "common/common.hpp"


Syntax::Production Syntax::BoostPrecedence( Syntax::Production prec )
{
	return (Syntax::Production)((int)prec + 1);
}


int Syntax::GetPrecedence( Syntax::Production prec )
{
	return (int)prec;
}
    
