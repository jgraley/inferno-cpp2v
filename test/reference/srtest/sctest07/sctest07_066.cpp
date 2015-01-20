#include "isystemc.h"

class TopLevel;
int gvar;
int i;
int j;
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
T_STATE_YIELD_1 = 4U,
T_STATE_PROCEED_NEXT = 1U,
T_STATE_PROCEED_THEN_ELSE = 5U,
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
T_STATE_PROCEED_NEXT_1 = 0U,
};
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_THEN_ELSE_YIELD_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT, &&PROCEED_THEN_ELSE_YIELD_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT, &&PROCEED_THEN_ELSE_YIELD_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT, &&PROCEED_THEN_ELSE_YIELD_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT, &&PROCEED_THEN_ELSE_YIELD_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT, &&PROCEED_THEN_ELSE_YIELD_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT };
auto unsigned int state;
PROCEED_THEN_ELSE_YIELD_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT:;
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
goto *(lmap[state]);
}
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
