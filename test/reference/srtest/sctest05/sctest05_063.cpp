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
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED, &&YIELD, &&THEN_ELSE, &&PROCEED_THEN_ELSE };
auto unsigned int state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(5))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
PROCEED_NEXT:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
 ::gvar+= ::i;
state=((!((0)==( ::i%(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
PROCEED:;
if(  ::TopLevel::T_STATE_PROCEED==state )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD:;
if(  ::TopLevel::T_STATE_YIELD==state )
{
 ::gvar^=(1);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
THEN_ELSE:;
if(  ::TopLevel::T_STATE_THEN_ELSE==state )
{
 ::gvar*=(2);
 ::i++;
state=(( ::i<(5)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
PROCEED_THEN_ELSE:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE==state )
{
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}
