#include "isystemc.h"

class TopLevel;
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
T_STATE_PROCEED_THEN_ELSE = 2U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_YIELD = 1U,
};
};
TopLevel top_level("top_level");
int i;
int gvar;

void TopLevel::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&YIELD, &&PROCEED_THEN_ELSE };
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(5))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD:;
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar*=(2);
 ::i++;
state=(( ::i<(5)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
goto *(lmap[state]);
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}
