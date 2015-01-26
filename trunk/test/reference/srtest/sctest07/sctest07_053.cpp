#include "isystemc.h"

class TopLevel;
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
T_STATE_YIELD = 2U,
T_STATE_YIELD1 = 4U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE = 5U,
T_STATE_PROCEED_NEXT1 = 1U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
};
};
TopLevel top_level("top_level");
int gvar;
int i;

void TopLevel::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED_NEXT1, &&YIELD, &&PROCEED_THEN_ELSE, &&YIELD1, &&PROCEED_THEN_ELSE1 };
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
}
goto *(lmap[state]);
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
YIELD:;
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
goto *(lmap[state]);
PROCEED_THEN_ELSE:;
 ::gvar*=(2);
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD1;
goto *(lmap[state]);
YIELD1:;
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
PROCEED_THEN_ELSE1:;
cease(  ::gvar );
return ;
}
