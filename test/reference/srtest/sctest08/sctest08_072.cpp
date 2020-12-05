#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_ENTER_helper = 4U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_LINK = 1U,
T_STATE_LINK1 = 8U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_NEXT1 = 5U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE1 = 7U,
T_STATE_YIELD = 2U,
T_STATE_YIELD1 = 6U,
};
void T();
private:
unsigned int helper_stack_index;
unsigned int link;
unsigned int (link_stack[10U]);
int (n_stack[10U]);
unsigned int state;
public:
/*temp*/ unsigned int helper_link;
/*temp*/ int helper_n;
/*temp*/ unsigned int otherhelper_link;
};
int gvar;
int i;
int j;
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
/*temp*/ unsigned int temp_link1;
/*temp*/ int temp_n;
do
{
if( (sc_delta_count())==(0U) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT== ::TopLevel::state )
{
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
continue;
}
if(  ::TopLevel::T_STATE_YIELD== ::TopLevel::state )
{
 ::i=((1)+ ::i);
 ::TopLevel::state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE== ::TopLevel::state )
{
cease(  ::gvar );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
if(  ::TopLevel::T_STATE_ENTER_helper== ::TopLevel::state )
{
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
 ::TopLevel::state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT1== ::TopLevel::state )
{
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD1;
continue;
}
if(  ::TopLevel::T_STATE_YIELD1== ::TopLevel::state )
{
 ::gvar=((1)+ ::gvar);
 ::j=((1)+ ::j);
 ::TopLevel::state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1== ::TopLevel::state )
{
 ::TopLevel::otherhelper_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_otherhelper;
}
if(  ::TopLevel::T_STATE_LINK1== ::TopLevel::state )
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
 ::TopLevel::state=temp_link;
}
if(  ::TopLevel::T_STATE_ENTER_otherhelper== ::TopLevel::state )
{
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link1= ::TopLevel::link;
 ::TopLevel::state=temp_link1;
}
wait(SC_ZERO_TIME);
}
while( true );
}
