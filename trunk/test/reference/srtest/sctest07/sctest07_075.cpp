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
T_STATE_YIELD = 2U,
T_STATE_PROCEED_THEN_ELSE = 5U,
T_STATE_YIELD1 = 4U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_PROCEED_NEXT1 = 1U,
};
void T();
};
int gvar;
int j;
int i;
TopLevel top_level("top_level");

void TopLevel::T()
{
if( (sc_delta_count())==(0U) )
{
 ::gvar=(1);
 ::i=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
return ;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar+= ::i;
 ::j=(0);
 ::TopLevel::state=((!( ::j<(3))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
return ;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar++;
 ::j++;
 ::TopLevel::state=(( ::j<(3)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 )
{
 ::gvar*=(2);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD1;
return ;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD1 )
{
 ::i++;
 ::TopLevel::state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
return ;
}
next_trigger(SC_ZERO_TIME);
}