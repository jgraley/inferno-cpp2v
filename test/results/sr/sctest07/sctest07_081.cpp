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
T_STATE_PROCEED_NEXT = 0,
T_STATE_PROCEED_NEXT_1 = 3,
T_STATE_PROCEED_THEN_ELSE = 1,
T_STATE_PROCEED_THEN_ELSE_1 = 5,
T_STATE_YIELD = 2,
T_STATE_YIELD_1 = 4,
};
void T();
};
TopLevel top_level("top_level");
int gvar;
int i;
int j;

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&YIELD, &&PROCEED_NEXT_1, &&YIELD_1, &&PROCEED_THEN_ELSE_1 };
auto unsigned int state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
PROCEED_NEXT:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
PROCEED_NEXT_1:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT_1==state )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD_1;
goto *(lmap[state]);
}
YIELD_1:;
if(  ::TopLevel::T_STATE_YIELD_1==state )
{
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
PROCEED_THEN_ELSE:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE==state )
{
 ::gvar*=(2);
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD:;
if(  ::TopLevel::T_STATE_YIELD==state )
{
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
PROCEED_THEN_ELSE_1:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1==state )
{
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}