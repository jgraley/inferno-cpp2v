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
void T();
/*temp*/ int helper_n;
private:
unsigned int (link_stack[10U]);
unsigned int link;
public:
/*temp*/ unsigned int otherhelper_link;
/*temp*/ unsigned int helper_link_1;
enum TStates
{
T_STATE_PROCEED_NEXT = 0U,
T_STATE_LINK = 1U,
T_STATE_PROCEED_THEN_ELSE = 7U,
T_STATE_YIELD = 6U,
T_STATE_LINK_1 = 8U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
T_STATE_ENTER_helper = 4U,
T_STATE_YIELD_1 = 2U,
T_STATE_PROCEED_NEXT_1 = 5U,
};
private:
int (n_stack[10U]);
public:
/*temp*/ unsigned int otherhelper_link_1;
private:
unsigned int helper_stack_index;
};
int j;
TopLevel top_level("top_level");
int i;
int gvar;

void TopLevel::T()
{
/*temp*/ int temp_n;
/*temp*/ unsigned int temp_link;
auto unsigned int state;
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&LINK, &&YIELD, &&PROCEED_THEN_ELSE, &&ENTER_helper, &&PROCEED_NEXT_1, &&YIELD_1, &&PROCEED_THEN_ELSE_1, &&LINK_1, &&ENTER_otherhelper };
/*temp*/ unsigned int temp_link_1;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
PROCEED_NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link_1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_helper;
goto *(lmap[state]);
LINK:;
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD_1;
goto *(lmap[state]);
YIELD:;
 ::i=( ::i+(1));
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
goto *(lmap[state]);
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
state= ::TopLevel::T_STATE_ENTER_helper;
goto *(lmap[state]);
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link_1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
goto *(lmap[state]);
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
YIELD_1:;
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
PROCEED_THEN_ELSE_1:;
 ::TopLevel::otherhelper_link_1= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_otherhelper;
goto *(lmap[state]);
LINK_1:;
temp_link_1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
state=temp_link_1;
goto *(lmap[state]);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link_1;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
state=temp_link;
goto *(lmap[state]);
}
