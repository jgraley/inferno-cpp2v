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
void method();
int gvar1;
int tot1;
};
TopLevel top_level("top_level");

void TopLevel::method()
{
 ::TopLevel::gvar1++;
 ::TopLevel::tot1+= ::TopLevel::gvar1;
if(  ::TopLevel::gvar1==(10) )
cease(  ::TopLevel::tot1 );
next_trigger(SC_ZERO_TIME);
}
