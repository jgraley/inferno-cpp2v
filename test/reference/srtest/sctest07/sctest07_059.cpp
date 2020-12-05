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
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_PROCEED_NEXT = 1U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE1 = 5U,
T_STATE_YIELD1 = 2U,
T_STATE_YIELD = 4U,
};
void T();
};
int gvar;
int i;
int j;
TopLevel top_level("top_level");

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED_NEXT1, &&YIELD, &&PROCEED_THEN_ELSE, &&YIELD1, &&PROCEED_THEN_ELSE1 };
auto unsigned int state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT1:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD1;
goto *(lmap[state]);
}
YIELD:;
if( state== ::TopLevel::T_STATE_YIELD1 )
{
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
PROCEED_THEN_ELSE:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
 ::gvar*=(2);
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD1:;
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
goto *(lmap[state]);
PROCEED_THEN_ELSE1:;
cease(  ::gvar );
return ;
}
