#include "isystemc.h"

class TopLevel;
int gvar = 0;
int tot = 0;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(method);
}
int tot_1;
int gvar_1;
void method();
};
TopLevel top_level("top_level");

void TopLevel::method()
{
 ::TopLevel::gvar_1++;
 ::TopLevel::tot_1+= ::TopLevel::gvar_1;
if( (10)== ::TopLevel::gvar_1 )
cease(  ::TopLevel::tot_1 );
next_trigger(SC_ZERO_TIME);
}
