#include "isystemc.h"

class TopLevel;
int tot = 0;
int gvar = 0;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(method);
}
void method();
int tot_1;
int gvar_1;
};
TopLevel top_level("top_level");

void TopLevel::method()
{
 ::TopLevel::gvar_1++;
 ::TopLevel::tot_1+= ::TopLevel::gvar_1;
if( (10)== ::TopLevel::gvar_1 )
cease(  ::TopLevel::tot_1 );
next_trigger(SC_ZERO_TIME);
return ;
}
