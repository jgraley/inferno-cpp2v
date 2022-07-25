#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0)
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_ENTER_helper = 2,
T_STATE_ENTER_otherhelper = 9,
T_STATE_LINK = 0,
T_STATE_LINK_1 = 8,
T_STATE_PROCEED_NEXT = 3,
T_STATE_PROCEED_NEXT_1 = 4,
T_STATE_PROCEED_THEN_ELSE = 1,
T_STATE_PROCEED_THEN_ELSE_1 = 7,
T_STATE_YIELD = 5,
T_STATE_YIELD_1 = 6,
};
void T();
private:
int (n_stack[10]);
unsigned int (link_stack[10]);
unsigned int helper_stack_index;
unsigned int link;
unsigned int state;
public:
/*temp*/ int helper_n;
/*temp*/ unsigned int helper_link;
/*temp*/ unsigned int otherhelper_link;
};
TopLevel top_level("top_level");
int gvar;
int i;
int j;

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
/*temp*/ unsigned int temp_link_1;
/*temp*/ int temp_n;
do
{
if( (sc_delta_count())==(0) )
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
 ::TopLevel::helper_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD_1;
continue;
}
if(  ::TopLevel::T_STATE_YIELD_1== ::TopLevel::state )
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
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
 ::j=(0);
 ::TopLevel::state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT_1== ::TopLevel::state )
{
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
continue;
}
if(  ::TopLevel::T_STATE_YIELD== ::TopLevel::state )
{
 ::gvar=((1)+ ::gvar);
 ::j=((1)+ ::j);
 ::TopLevel::state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1== ::TopLevel::state )
{
 ::TopLevel::otherhelper_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_otherhelper;
}
if(  ::TopLevel::T_STATE_LINK_1== ::TopLevel::state )
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
 ::TopLevel::state=temp_link;
}
if(  ::TopLevel::T_STATE_ENTER_otherhelper== ::TopLevel::state )
{
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link_1= ::TopLevel::link;
 ::TopLevel::state=temp_link_1;
}
wait(SC_ZERO_TIME);
}
while( true );
}
