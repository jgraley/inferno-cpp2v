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
T_STATE_PROCEED = 1U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE = 4U,
T_STATE_THEN_ELSE = 3U,
T_STATE_YIELD = 2U,
};
void T();
};
int gvar;
int i;
TopLevel top_level("top_level");

void TopLevel::T()
{
static const void *(lmap[]) = { &&PROCEED_NEXT, &&PROCEED, &&YIELD, &&THEN_ELSE, &&PROCEED_THEN_ELSE };
auto void *state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(5))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
 ::gvar+= ::i;
{
state=((!(( ::i%(2))==(0))) ? (lmap[ ::TopLevel::T_STATE_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED:;
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::gvar^=(1);
{
state=(lmap[ ::TopLevel::T_STATE_THEN_ELSE]);
goto *(state);
}
THEN_ELSE:;
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
