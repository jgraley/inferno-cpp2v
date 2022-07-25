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
T_STATE_PROCEED_NEXT = 1,
T_STATE_PROCEED_THEN_ELSE = 2,
T_STATE_YIELD = 0,
};
void T();
private:
int t;
};
TopLevel top_level("top_level");
int gvar;
int i;

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&YIELD, &&PROCEED_NEXT, &&PROCEED_THEN_ELSE };
auto unsigned int state;
 ::gvar=(1);
 ::TopLevel::t=(5);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i< ::TopLevel::t)) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
PROCEED_NEXT:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD:;
if(  ::TopLevel::T_STATE_YIELD==state )
{
 ::gvar*=(2);
 ::i++;
state=(( ::i< ::TopLevel::t) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
PROCEED_THEN_ELSE:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE==state )
{
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}
