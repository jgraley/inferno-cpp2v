#include "isystemc.h"

class TopLevel;
int gvar;
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
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_YIELD = 2U,
T_STATE_PROCEED_THEN_ELSE_1 = 4U,
T_STATE_PROCEED_NEXT_1 = 1U,
};
};
TopLevel top_level("top_level");
int i;

void TopLevel::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&PROCEED_NEXT_1, &&PROCEED_NEXT, &&YIELD, &&PROCEED_THEN_ELSE_1, &&PROCEED_THEN_ELSE };
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
PROCEED_THEN_ELSE:;
PROCEED_THEN_ELSE_1:;
YIELD:;
PROCEED_NEXT:;
PROCEED_NEXT_1:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT_1 )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE==state )
{
 ::gvar*=(2);
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 )
{
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}
