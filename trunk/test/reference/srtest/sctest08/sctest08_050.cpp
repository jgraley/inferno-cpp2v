#include "isystemc.h"

class TopLevel;
int gvar;
int i;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
enum TStates
{
T_STATE_YIELD = 6U,
T_STATE_YIELD_1 = 2U,
T_STATE_ENTER_helper = 4U,
T_STATE_LINK = 8U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_PROCEED_THEN_ELSE = 7U,
T_STATE_PROCEED_NEXT = 5U,
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
T_STATE_LINK_1 = 1U,
T_STATE_PROCEED_NEXT_1 = 0U,
};
void T();
private:
unsigned int helper_stack_index;
public:
/*temp*/ void *otherhelper_link_1;
/*temp*/ void *helper_link_1;
/*temp*/ int helper_n;
private:
void *(link_stack[10U]);
int (n_stack[10U]);
void *link;
};
TopLevel top_level("top_level");
int j;

void TopLevel::T()
{
static const void *(lmap[]) = { &&PROCEED_NEXT, &&LINK, &&YIELD, &&PROCEED_THEN_ELSE, &&ENTER_helper, &&PROCEED_NEXT_1, &&YIELD_1, &&PROCEED_THEN_ELSE_1, &&LINK_1, &&ENTER_otherhelper };
/*temp*/ int temp_n;
/*temp*/ void *temp_link;
/*temp*/ void *temp_link_1;
auto void *state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]));
goto *(state);
}
PROCEED_NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link=(lmap[ ::TopLevel::T_STATE_LINK_1]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_helper]);
goto *(state);
}
LINK:;
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD_1]);
goto *(state);
}
YIELD:;
 ::i=( ::i+(1));
{
state=(( ::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]));
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
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
{
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD]);
goto *(state);
}
YIELD_1:;
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
{
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::TopLevel::otherhelper_link=(lmap[ ::TopLevel::T_STATE_LINK]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_otherhelper]);
goto *(state);
}
LINK_1:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
{
state=temp_link;
goto *(state);
}
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link_1= ::TopLevel::link;
{
state=temp_link_1;
goto *(state);
}
}
