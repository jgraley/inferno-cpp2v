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
T_STATE_PROCEED_NEXT_1 = 1,
T_STATE_PROCEED_THEN_ELSE = 2,
T_STATE_PROCEED_THEN_ELSE_1 = 4,
T_STATE_YIELD = 3,
};
void T();
};
TopLevel top_level("top_level");
int gvar;
int i;
int j;

void TopLevel::T()
{
static const void *(lmap[]) = { &&PROCEED_NEXT, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE, &&YIELD, &&PROCEED_THEN_ELSE_1 };
auto void *state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]));
goto *(state);
}
PROCEED_NEXT_1:;
 ::gvar+= ::i;
 ::j=(0);
{
state=((!( ::j<(3))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::gvar++;
 ::j++;
{
state=(( ::j<(3)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::gvar*=(2);
 ::i++;
{
state=(( ::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}
