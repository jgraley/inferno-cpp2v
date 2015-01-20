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
T_STATE_YIELD = 2U,
T_STATE_PROCEED_NEXT = 1U,
T_STATE_YIELD_1 = 4U,
T_STATE_PROCEED_NEXT_1 = 0U,
T_STATE_PROCEED_THEN_ELSE = 5U,
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
};
void T();
};
int i;
int gvar;
TopLevel top_level("top_level");
int j;

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT_1, &&PROCEED_NEXT, &&YIELD_1, &&PROCEED_THEN_ELSE_1, &&YIELD, &&PROCEED_THEN_ELSE };
auto unsigned int state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
PROCEED_THEN_ELSE:;
YIELD:;
PROCEED_THEN_ELSE_1:;
YIELD_1:;
PROCEED_NEXT:;
PROCEED_NEXT_1:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT_1==state )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 )
{
 ::gvar*=(2);
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD_1;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD_1 )
{
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}
