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
T_STATE_PROCEED = 1,
T_STATE_PROCEED_NEXT = 2,
T_STATE_PROCEED_THEN_ELSE = 4,
T_STATE_THEN_ELSE = 0,
T_STATE_YIELD = 3,
};
void T();
};
TopLevel top_level("top_level");
int gvar;
int i;

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&THEN_ELSE, &&PROCEED, &&PROCEED_NEXT, &&YIELD, &&PROCEED_THEN_ELSE };
auto unsigned int state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(5))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
}
PROCEED_NEXT:;
 ::gvar+= ::i;
{
state=((!((0)==( ::i%(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
}
PROCEED:;
wait(SC_ZERO_TIME);
{
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD:;
 ::gvar^=(1);
{
state= ::TopLevel::T_STATE_THEN_ELSE;
goto *(lmap[state]);
}
THEN_ELSE:;
 ::gvar*=(2);
 ::i++;
{
state=(( ::i<(5)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}
