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
T_STATE_YIELD = 2U,
T_STATE_PROCEED_THEN_ELSE = 4U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_THEN_ELSE = 3U,
T_STATE_PROCEED = 1U,
};
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_THEN_ELSE_THEN_ELSE_YIELD_PROCEED_PROCEED_NEXT, &&PROCEED_THEN_ELSE_THEN_ELSE_YIELD_PROCEED_PROCEED_NEXT, &&PROCEED_THEN_ELSE_THEN_ELSE_YIELD_PROCEED_PROCEED_NEXT, &&PROCEED_THEN_ELSE_THEN_ELSE_YIELD_PROCEED_PROCEED_NEXT, &&PROCEED_THEN_ELSE_THEN_ELSE_YIELD_PROCEED_PROCEED_NEXT };
auto unsigned int state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(5))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
PROCEED_THEN_ELSE_THEN_ELSE_YIELD_PROCEED_PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar+= ::i;
state=((!((0)==( ::i%(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
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