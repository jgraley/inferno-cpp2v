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
T_STATE_LINK1 = 1U,
T_STATE_LINK = 8U,
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
void *link;
void *(link_stack[10U]);
int (n_stack[10U]);
public:
/*temp*/ void *helper_link;
/*temp*/ void *helper_link1;
/*temp*/ int helper_n;
/*temp*/ void *otherhelper_link1;
/*temp*/ void *otherhelper_link;
};
int gvar;
int i;
int j;
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link1;
/*temp*/ void *temp_link;
static const void *(lmap[]) = { &&PROCEED_NEXT, &&LINK, &&YIELD, &&PROCEED_THEN_ELSE, &&ENTER_helper, &&PROCEED_NEXT1, &&YIELD1, &&PROCEED_THEN_ELSE1, &&LINK1, &&ENTER_otherhelper };
auto void *state;
/*temp*/ int temp_n;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link1=(lmap[ ::TopLevel::T_STATE_LINK1]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_helper]);
goto *(state);
}
LINK:;
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::i=( ::i+(1));
{
state=(( ::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_helper]);
goto *(state);
}
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
{
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT1]));
goto *(state);
}
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD1]);
goto *(state);
}
YIELD1:;
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
{
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE1]));
goto *(state);
}
PROCEED_THEN_ELSE1:;
 ::TopLevel::otherhelper_link=(lmap[ ::TopLevel::T_STATE_LINK]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_otherhelper]);
goto *(state);
}
LINK1:;
temp_link1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
{
state=temp_link1;
goto *(state);
}
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
