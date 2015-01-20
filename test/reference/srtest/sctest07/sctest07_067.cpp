#include "isystemc.h"

class TopLevel;
int i;
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
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE = 5U,
T_STATE_YIELD = 4U,
T_STATE_PROCEED_NEXT_1 = 1U,
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
T_STATE_YIELD_1 = 2U,
};
};
TopLevel top_level("top_level");
int j;
int gvar;

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&, &&, &&, &&, &&, && };
auto unsigned int state;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT_1==state )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD_1;
continue;
}
if( state== ::TopLevel::T_STATE_YIELD_1 )
{
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 )
{
 ::gvar*=(2);
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
continue;
}
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
return ;
}
}
while( true );
}
