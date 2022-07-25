#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_PROCEED_NEXT = 1,
T_STATE_PROCEED_THEN_ELSE = 2,
T_STATE_YIELD = 0,
};
void T();
private:
int t;
unsigned int state;
};
TopLevel top_level("top_level");
int gvar;
int i;

void TopLevel::T()
{
do
{
if( (sc_delta_count())==(0) )
{
 ::gvar=(1);
 ::TopLevel::t=(5);
 ::i=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i< ::TopLevel::t)) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT== ::TopLevel::state )
{
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
continue;
}
if(  ::TopLevel::T_STATE_YIELD== ::TopLevel::state )
{
 ::gvar*=(2);
 ::i++;
 ::TopLevel::state=(( ::i< ::TopLevel::t) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE== ::TopLevel::state )
{
cease(  ::gvar );
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}
