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
static const void *(lmap[]) = { &&YIELD, &&PROCEED_NEXT, &&PROCEED_THEN_ELSE };
auto void *state;
 ::gvar=(1);
 ::TopLevel::t=(5);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i< ::TopLevel::t)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::gvar*=(2);
 ::i++;
{
state=(( ::i< ::TopLevel::t) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}
