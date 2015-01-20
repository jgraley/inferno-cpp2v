#include "isystemc.h"

class TopLevel;
int j;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
void T();
private:
unsigned int state;
public:
enum TStates
{
T_STATE_PROCEED_NEXT = 0U,
T_STATE_YIELD = 2U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE_1 = 5U,
T_STATE_PROCEED_NEXT_1 = 1U,
T_STATE_YIELD_1 = 4U,
};
};
TopLevel top_level("top_level");
int gvar;
int i;

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
enabled=(false);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_NEXT== ::TopLevel::state) )
{
 ::gvar+= ::i;
 ::j=(0);
 ::TopLevel::state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_NEXT_1== ::TopLevel::state) )
{
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_YIELD) )
{
 ::gvar++;
 ::j++;
 ::TopLevel::state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE) )
{
 ::gvar*=(2);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD_1;
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_YIELD_1) )
{
 ::i++;
 ::TopLevel::state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1) )
{
cease(  ::gvar );
enabled=(false);
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
