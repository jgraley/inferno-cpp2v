#include "isystemc.h"

class TopLevel;
int gvar;
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
T_STATE_PROCEED_NEXT = 5U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_YIELD = 6U,
T_STATE_ENTER_helper = 4U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_PROCEED_THEN_ELSE1 = 7U,
T_STATE_LINK = 8U,
T_STATE_LINK1 = 1U,
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_YIELD1 = 2U,
};
/*temp*/ unsigned int otherhelper_link;
/*temp*/ int helper_n;
/*temp*/ unsigned int otherhelper_link1;
private:
unsigned int (link_stack[10U]);
public:
/*temp*/ unsigned int helper_link;
/*temp*/ unsigned int helper_link1;
void T();
private:
int (n_stack[10U]);
unsigned int link;
unsigned int helper_stack_index;
};
int j;
TopLevel top_level("top_level");
int i;

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&LINK, &&YIELD, &&PROCEED_THEN_ELSE, &&ENTER_helper, &&PROCEED_NEXT1, &&YIELD1, &&PROCEED_THEN_ELSE1, &&LINK1, &&ENTER_otherhelper };
/*temp*/ int temp_n;
auto unsigned int state;
/*temp*/ unsigned int temp_link;
/*temp*/ unsigned int temp_link1;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT1);
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_helper;
}
LINK:;
if( state== ::TopLevel::T_STATE_LINK1 )
{
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD1;
goto *(lmap[state]);
}
YIELD:;
if( state== ::TopLevel::T_STATE_YIELD1 )
{
 ::i=((1)+ ::i);
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
PROCEED_THEN_ELSE:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
return ;
state= ::TopLevel::T_STATE_ENTER_helper;
}
ENTER_helper:;
if( state== ::TopLevel::T_STATE_ENTER_helper )
{
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT1:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
goto *(lmap[state]);
YIELD1:;
 ::gvar=( ::gvar+(1));
 ::j=((1)+ ::j);
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
goto *(lmap[state]);
PROCEED_THEN_ELSE1:;
 ::TopLevel::otherhelper_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_otherhelper;
goto *(lmap[state]);
LINK1:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
state=temp_link;
goto *(lmap[state]);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link1= ::TopLevel::link;
state=temp_link1;
goto *(lmap[state]);
}