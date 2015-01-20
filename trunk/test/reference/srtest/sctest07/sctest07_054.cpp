#include "isystemc.h"

class TopLevel;
int j;
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
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_YIELD = 2U,
T_STATE_PROCEED_NEXT = 1U,
T_STATE_PROCEED_THEN_ELSE_1 = 5U,
T_STATE_YIELD_1 = 4U,
T_STATE_PROCEED_NEXT_1 = 0U,
};
};
TopLevel top_level("top_level");
int gvar;

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED_NEXT_1, &&YIELD, &&PROCEED_THEN_ELSE, &&YIELD_1, &&PROCEED_THEN_ELSE_1 };
auto unsigned int state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
PROCEED_NEXT:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT_1==state )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT_1:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
goto *(lmap[state]);
YIELD:;
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
PROCEED_THEN_ELSE:;
 ::gvar*=(2);
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD_1;
goto *(lmap[state]);
YIELD_1:;
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
goto *(lmap[state]);
PROCEED_THEN_ELSE_1:;
cease(  ::gvar );
return ;
}
