#include "isystemc.h"

class TopLevel;
int i;
int gvar;
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
TopLevel top_level("top_level");

void TopLevel::T()
{
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(5))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
return ;
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT== ::TopLevel::state )
{
 ::gvar+= ::i;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
return ;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar*=(2);
 ::i++;
 ::TopLevel::state=(( ::i<(5)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
return ;
}
next_trigger(SC_ZERO_TIME);
}
