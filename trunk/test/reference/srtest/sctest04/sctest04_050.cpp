#include "isystemc.h"

class TopLevel;
int gvar;
int i;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_PROCEED_NEXT = 0U,
T_STATE_YIELD = 1U,
T_STATE_PROCEED_THEN_ELSE = 2U,
};
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto void *state;
static const void *(lmap[]) = { &&PROCEED_NEXT, &&YIELD, &&PROCEED_THEN_ELSE };
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(5))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::gvar*=(2);
 ::i++;
{
state=(( ::i<(5)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}