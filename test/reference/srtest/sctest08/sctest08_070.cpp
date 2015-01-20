#include "isystemc.h"

class TopLevel;
int i;
int j;
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
T_STATE_ENTER_otherhelper = 9U,
T_STATE_ENTER_helper = 4U,
T_STATE_PROCEED_NEXT = 5U,
T_STATE_YIELD = 6U,
T_STATE_PROCEED_THEN_ELSE = 7U,
T_STATE_YIELD_1 = 2U,
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
T_STATE_LINK = 1U,
T_STATE_PROCEED_NEXT_1 = 0U,
T_STATE_LINK_1 = 8U,
};
private:
unsigned int (link_stack[10U]);
unsigned int helper_stack_index;
public:
/*temp*/ unsigned int helper_link;
/*temp*/ unsigned int otherhelper_link;
/*temp*/ int helper_n;
private:
int (n_stack[10U]);
unsigned int link;
public:
void T();
};
TopLevel top_level("top_level");
int gvar;

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ int temp_n;
/*temp*/ unsigned int temp_link;
/*temp*/ unsigned int temp_link_1;
do
{
if( (sc_delta_count())==(0U) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
continue;
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT_1 )
{
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_helper;
}
if( state== ::TopLevel::T_STATE_LINK )
{
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD_1;
continue;
}
if( state== ::TopLevel::T_STATE_YIELD_1 )
{
 ::i=((1)+ ::i);
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 )
{
cease(  ::gvar );
return ;
state= ::TopLevel::T_STATE_ENTER_helper;
}
if( state== ::TopLevel::T_STATE_ENTER_helper )
{
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
continue;
}
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
 ::TopLevel::otherhelper_link= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_otherhelper;
}
if( state== ::TopLevel::T_STATE_LINK_1 )
{
temp_link_1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
state=temp_link_1;
}
if( state== ::TopLevel::T_STATE_ENTER_otherhelper )
{
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
