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
T_STATE_PROCEED_THEN_ELSE = 4U,
T_STATE_PROCEED_NEXT = 1U,
T_STATE_YIELD = 2U,
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
T_STATE_PROCEED_NEXT_1 = 0U,
};
};
int j;
int i;
TopLevel top_level("top_level");
int gvar;

void TopLevel::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED_NEXT_1, &&YIELD, &&PROCEED_THEN_ELSE, &&PROCEED_THEN_ELSE_1 };
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
goto *(lmap[state]);
}
PROCEED_NEXT:;
 ::gvar+= ::i;
 ::j=(0);
{
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
}
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
{
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD:;
 ::gvar++;
 ::j++;
{
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE:;
 ::gvar*=(2);
 ::i++;
{
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE_1:;
cease(  ::gvar );
return ;
}
