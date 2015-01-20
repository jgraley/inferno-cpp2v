#include "isystemc.h"

class TopLevel;
int j;
int i;
int gvar;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
private:
unsigned int link;
public:
/*temp*/ unsigned int temp_link;
private:
unsigned int (link_stack[10U]);
public:
void T();
/*temp*/ int temp_n;
/*temp*/ unsigned int temp_link_1;
/*temp*/ unsigned int otherhelper_link;
/*temp*/ int helper_n;
private:
unsigned int state;
int (n_stack[10U]);
public:
enum TStates
{
T_STATE_LINK = 8U,
T_STATE_YIELD = 2U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_NEXT_1 = 5U,
T_STATE_LINK_1 = 1U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_YIELD_1 = 6U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_ENTER_helper = 4U,
T_STATE_PROCEED_THEN_ELSE_1 = 7U,
};
private:
unsigned int helper_stack_index;
public:
/*temp*/ unsigned int helper_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
do
{
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar=( ::gvar+ ::i);
 ::TopLevel::temp_n=(3);
 ::TopLevel::helper_n= ::TopLevel::temp_n;
 ::TopLevel::helper_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK_1 )
{
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD )
{
 ::i=( ::i+(1));
 ::TopLevel::state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
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
 ::TopLevel::state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_1 )
{
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD_1;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD_1 )
{
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
 ::TopLevel::state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 )
{
 ::TopLevel::otherhelper_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_otherhelper;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_otherhelper )
{
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
 ::TopLevel::temp_link_1= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link_1;
}
wait(SC_ZERO_TIME);
}
while( true );
}
