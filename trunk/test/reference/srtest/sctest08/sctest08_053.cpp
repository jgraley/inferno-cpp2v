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
/*temp*/ unsigned int helper_link;
private:
unsigned int (link_stack[10U]);
public:
void T();
enum TStates
{
T_STATE_PROCEED_NEXT = 5U,
T_STATE_YIELD = 6U,
T_STATE_LINK = 1U,
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_PROCEED_THEN_ELSE = 7U,
T_STATE_LINK1 = 8U,
T_STATE_YIELD1 = 2U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_ENTER_helper = 4U,
};
/*temp*/ unsigned int otherhelper_link;
/*temp*/ int helper_n;
/*temp*/ unsigned int otherhelper_link1;
/*temp*/ unsigned int helper_link1;
private:
unsigned int link;
unsigned int helper_stack_index;
int (n_stack[10U]);
};
TopLevel top_level("top_level");
int gvar;

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&LINK, &&YIELD, &&PROCEED_THEN_ELSE, &&ENTER_helper, &&PROCEED_NEXT1, &&YIELD1, &&PROCEED_THEN_ELSE1, &&LINK1, &&ENTER_otherhelper };
auto unsigned int state;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_n;
/*temp*/ unsigned int temp_link1;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_helper;
}
goto *(lmap[state]);
LINK:;
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD1;
goto *(lmap[state]);
YIELD:;
 ::i=( ::i+(1));
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
goto *(lmap[state]);
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
state= ::TopLevel::T_STATE_ENTER_helper;
goto *(lmap[state]);
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
YIELD1:;
 ::gvar=( ::gvar+(1));
 ::j=((1)+ ::j);
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
PROCEED_THEN_ELSE1:;
 ::TopLevel::otherhelper_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_otherhelper;
goto *(lmap[state]);
LINK1:;
temp_link1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
state=temp_link1;
goto *(lmap[state]);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
state=temp_link;
goto *(lmap[state]);
}