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
TopLevel(auto char (*name));
int gvar;
int tot;
};
TopLevel top_level("top_level");
void SC_ZERO_TIME;
int gvar_1 = 0;
int tot_1 = 0;
void (cease)(void p1);
void (exit)(void p1_1);
void (next_trigger)(void p1_2);
void (wait)(void p1_3);

void TopLevel::method()
{
 ::TopLevel::gvar++;
 ::TopLevel::tot+= ::TopLevel::gvar;
if( (10)== ::TopLevel::gvar )
cease(  ::TopLevel::tot );
next_trigger(SC_ZERO_TIME);
}

TopLevel::TopLevel(char (*name))
{
}

void (cease)(void p1);

void (exit)(void p1_1);

void (next_trigger)(void p1_2);

void (wait)(void p1_3);
