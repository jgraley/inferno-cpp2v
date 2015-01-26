#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
enum TStates
{
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE = 4U,
T_STATE_YIELD = 2U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_PROCEED_NEXT1 = 1U,
};
void T();
private:
unsigned int state;
};
int gvar;
int j;
TopLevel top_level("top_level");
int i;

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT) )
{
 ::gvar+= ::i;
 ::j=(0);
 ::TopLevel::state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT1) )
{
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_YIELD) )
{
 ::gvar++;
 ::j++;
 ::TopLevel::state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE1) )
{
 ::gvar*=(2);
 ::i++;
 ::TopLevel::state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE) )
{
cease(  ::gvar );
enabled=(false);
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
