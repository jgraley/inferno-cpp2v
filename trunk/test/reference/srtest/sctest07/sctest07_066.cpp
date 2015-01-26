#include "isystemc.h"

class TopLevel;
int i;
int gvar;
int j;
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
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_YIELD = 4U,
T_STATE_YIELD1 = 2U,
T_STATE_PROCEED_THEN_ELSE1 = 5U,
T_STATE_PROCEED_NEXT1 = 1U,
};
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
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT1);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD1;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD1 )
{
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
 ::gvar*=(2);
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 )
{
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}
