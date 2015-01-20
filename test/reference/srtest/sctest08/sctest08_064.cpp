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
/*temp*/ unsigned int helper_link;
private:
unsigned int helper_stack_index;
public:
/*temp*/ unsigned int otherhelper_link;
private:
unsigned int link;
int (n_stack[10U]);
public:
enum TStates
{
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_PROCEED_NEXT_1 = 5U,
T_STATE_LINK = 1U,
T_STATE_ENTER_helper = 4U,
T_STATE_PROCEED_THEN_ELSE_1 = 7U,
T_STATE_LINK_1 = 8U,
T_STATE_YIELD = 2U,
T_STATE_YIELD_1 = 6U,
};
/*temp*/ unsigned int otherhelper_link_1;
/*temp*/ unsigned int helper_link_1;
void T();
/*temp*/ int helper_n;
private:
unsigned int (link_stack[10U]);
};
int j;
int i;
int gvar;
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&PROCEED_NEXT_1, &&LINK_1, &&YIELD_1, &&PROCEED_THEN_ELSE_1, &&ENTER_helper, &&PROCEED_NEXT, &&YIELD, &&PROCEED_THEN_ELSE, &&LINK, &&ENTER_otherhelper };
/*temp*/ int temp_n;
/*temp*/ unsigned int temp_link_1;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
ENTER_otherhelper:;
LINK:;
PROCEED_THEN_ELSE:;
YIELD:;
PROCEED_NEXT:;
ENTER_helper:;
PROCEED_THEN_ELSE_1:;
YIELD_1:;
LINK_1:;
PROCEED_NEXT_1:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link_1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_helper;
}
if( state== ::TopLevel::T_STATE_LINK )
{
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::i=( ::i+(1));
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
return ;
state= ::TopLevel::T_STATE_ENTER_helper;
}
if( state== ::TopLevel::T_STATE_ENTER_helper )
{
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link_1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT_1 )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD_1;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD_1 )
{
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 )
{
 ::TopLevel::otherhelper_link_1= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_otherhelper;
}
if( state== ::TopLevel::T_STATE_LINK_1 )
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
state=temp_link;
}
if( state== ::TopLevel::T_STATE_ENTER_otherhelper )
{
 ::TopLevel::link= ::TopLevel::otherhelper_link_1;
 ::gvar=( ::gvar-(1));
temp_link_1= ::TopLevel::link;
state=temp_link_1;
}
goto *(lmap[state]);
}
