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
T_STATE_PROCEED = 1,
T_STATE_PROCEED_NEXT = 2,
T_STATE_PROCEED_THEN_ELSE = 4,
T_STATE_THEN_ELSE = 0,
T_STATE_YIELD = 3,
};
void T();
};
TopLevel top_level("top_level");
int gvar;
int i;

void TopLevel::T()
{
auto unsigned int state;
do
{
if( (sc_delta_count())==(0) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(5))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
 ::gvar+= ::i;
state=((!((0)==( ::i%(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if(  ::TopLevel::T_STATE_PROCEED==state )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
continue;
}
if(  ::TopLevel::T_STATE_YIELD==state )
{
 ::gvar^=(1);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if(  ::TopLevel::T_STATE_THEN_ELSE==state )
{
 ::gvar*=(2);
 ::i++;
state=(( ::i<(5)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE==state )
{
cease(  ::gvar );
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}