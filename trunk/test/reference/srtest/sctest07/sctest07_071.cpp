#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
enum TStates
{
T_STATE_PROCEED_THEN_ELSE = 5U,
T_STATE_PROCEED_NEXT = 1U,
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_YIELD = 2U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_YIELD1 = 4U,
};
};
TopLevel top_level("top_level");
int j;
int gvar;
int i;

void TopLevel::T()
{
auto unsigned int state;
do
{
if( (sc_delta_count())==(0U) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT1);
continue;
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
continue;
}
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 )
{
 ::gvar*=(2);
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD1;
continue;
}
if( state== ::TopLevel::T_STATE_YIELD1 )
{
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}