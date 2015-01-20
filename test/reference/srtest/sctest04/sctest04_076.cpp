#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
private:
unsigned int state;
public:
enum TStates
{
T_STATE_PROCEED_THEN_ELSE = 2U,
T_STATE_YIELD = 1U,
T_STATE_PROCEED_NEXT = 0U,
};
void T();
};
int i;
TopLevel top_level("top_level");
int gvar;

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(5))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
enabled=(false);
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar+= ::i;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
enabled=(false);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar*=(2);
 ::i++;
 ::TopLevel::state=(( ::i<(5)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
enabled=(false);
}
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
