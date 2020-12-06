#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(method);
}
void method();
int gvar;
int tot;
};
TopLevel top_level("top_level");
int gvar1 = 0;
int tot1 = 0;

void TopLevel::method()
{
 ::TopLevel::gvar++;
 ::TopLevel::tot+= ::TopLevel::gvar;
if( (10)== ::TopLevel::gvar )
cease(  ::TopLevel::tot );
next_trigger(SC_ZERO_TIME);
return ;
}
