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
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_NEXT1 = 1U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE1 = 4U,
T_STATE_YIELD = 2U,
};
void T();
private:
unsigned int state;
};
int gvar;
int i;
int j;
TopLevel top_level("top_level");

void TopLevel::T()
{
do
{
if( (sc_delta_count())==(0U) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT== ::TopLevel::state )
{
 ::gvar+= ::i;
 ::j=(0);
 ::TopLevel::state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT1);
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT1== ::TopLevel::state )
{
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
continue;
}
if(  ::TopLevel::T_STATE_YIELD== ::TopLevel::state )
{
 ::gvar++;
 ::j++;
 ::TopLevel::state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE== ::TopLevel::state )
{
 ::gvar*=(2);
 ::i++;
 ::TopLevel::state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1== ::TopLevel::state )
{
cease(  ::gvar );
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}
