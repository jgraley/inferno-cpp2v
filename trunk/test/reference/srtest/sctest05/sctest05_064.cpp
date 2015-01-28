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
T_STATE_PROCEED = 1U,
T_STATE_YIELD = 2U,
T_STATE_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE = 4U,
};
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED, &&YIELD, &&THEN_ELSE, &&PROCEED_THEN_ELSE };
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(5))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
PROCEED_THEN_ELSE:;
THEN_ELSE:;
YIELD:;
PROCEED:;
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar+= ::i;
state=((!(( ::i%(2))==(0))) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if( state== ::TopLevel::T_STATE_PROCEED )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar^=(1);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
 ::gvar*=(2);
 ::i++;
state=(( ::i<(5)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}
