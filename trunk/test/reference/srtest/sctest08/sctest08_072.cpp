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
private:
unsigned int (link_stack[10U]);
public:
/*temp*/ unsigned int helper_link;
private:
unsigned int link;
int (n_stack[10U]);
public:
/*temp*/ int helper_n;
enum TStates
{
T_STATE_PROCEED_NEXT = 5U,
T_STATE_LINK = 1U,
T_STATE_PROCEED_THEN_ELSE = 7U,
T_STATE_ENTER_helper = 4U,
T_STATE_YIELD = 2U,
T_STATE_YIELD1 = 6U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_LINK1 = 8U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_PROCEED_NEXT1 = 0U,
};
void T();
private:
unsigned int state;
unsigned int helper_stack_index;
public:
/*temp*/ unsigned int otherhelper_link;
};
int j;
int gvar;
TopLevel top_level("top_level");
int i;

void TopLevel::T()
{
/*temp*/ int temp_n;
/*temp*/ unsigned int temp_link;
/*temp*/ unsigned int temp_link1;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD )
{
 ::i=( ::i+(1));
 ::TopLevel::state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 )
{
cease(  ::gvar );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_helper )
{
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
 ::TopLevel::state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD1;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD1 )
{
 ::gvar=( ::gvar+(1));
 ::j=((1)+ ::j);
 ::TopLevel::state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
 ::TopLevel::otherhelper_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_otherhelper;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
temp_link1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
 ::TopLevel::state=temp_link1;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_otherhelper )
{
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
 ::TopLevel::state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
