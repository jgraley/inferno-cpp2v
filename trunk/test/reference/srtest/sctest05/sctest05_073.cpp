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
private:
unsigned int state;
public:
enum TStates
{
T_STATE_PROCEED = 1U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_THEN_ELSE = 3U,
T_STATE_YIELD = 2U,
T_STATE_PROCEED_THEN_ELSE = 4U,
};
};
int gvar;
TopLevel top_level("top_level");
int i;

void TopLevel::T()
{
do
{
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(5))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar+= ::i;
 ::TopLevel::state=((!(( ::i%(2))==(0))) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar^=(1);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN_ELSE )
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
wait(SC_ZERO_TIME);
}
while( true );
}