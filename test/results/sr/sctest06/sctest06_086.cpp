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
T_STATE_PROCEED_NEXT = 0,
T_STATE_PROCEED_NEXT_1 = 1,
T_STATE_PROCEED_THEN_ELSE = 2,
T_STATE_PROCEED_THEN_ELSE_1 = 4,
T_STATE_YIELD = 3,
};
void T();
};
TopLevel top_level("top_level");
int gvar;
int i;
int j;

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT, &&PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT, &&PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT, &&PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT, &&PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_PROCEED_NEXT };
auto unsigned int state;
do
{
if( (sc_delta_count())==(0) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT_1==state )
{
 ::gvar+= ::i;
 ::j=(0);
state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
continue;
}
if(  ::TopLevel::T_STATE_YIELD==state )
{
 ::gvar++;
 ::j++;
state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1==state )
{
 ::gvar*=(2);
 ::i++;
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE==state )
{
cease(  ::gvar );
return ;
}
}
while( true );
}