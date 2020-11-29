#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(method);
}
int gvar1;
void method();
int tot1;
};
int gvar = 0;
TopLevel top_level("top_level");
int tot = 0;

void TopLevel::method()
{
 ::TopLevel::gvar1++;
 ::TopLevel::tot1+= ::TopLevel::gvar1;
if(  ::TopLevel::gvar1==(10) )
cease(  ::TopLevel::tot1 );
next_trigger(SC_ZERO_TIME);
return ;
}
