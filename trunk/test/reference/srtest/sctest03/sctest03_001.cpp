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
int tot1;
void method();
int gvar1;
};
TopLevel top_level("top_level");

void TopLevel::method()
{
 ::TopLevel::gvar1++;
 ::TopLevel::tot1+= ::TopLevel::gvar1;
if( (10)== ::TopLevel::gvar1 )
cease(  ::TopLevel::tot1 );
next_trigger(SC_ZERO_TIME);
}