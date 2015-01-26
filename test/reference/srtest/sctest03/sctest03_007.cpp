#include "isystemc.h"

class TopLevel;
int tot = 0;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(method);
}
int tot1;
int gvar1;
void method();
};
TopLevel top_level("top_level");
int gvar = 0;

void TopLevel::method()
{
 ::TopLevel::gvar1++;
 ::TopLevel::tot1+= ::TopLevel::gvar1;
if( (10)== ::TopLevel::gvar1 )
cease(  ::TopLevel::tot1 );
next_trigger(SC_ZERO_TIME);
}
