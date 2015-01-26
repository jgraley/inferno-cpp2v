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
void T();
enum TStates
{
T_STATE_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE = 4U,
T_STATE_PROCEED = 1U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_YIELD = 2U,
};
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto void *state;
static const void *(lmap[]) = { &&PROCEED_NEXT, &&PROCEED, &&YIELD, &&THEN_ELSE, &&PROCEED_THEN_ELSE };
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
state=((!((0)==( ::i%(2)))) ? (lmap[ ::TopLevel::T_STATE_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
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
