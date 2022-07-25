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
void *(link_stack[10]);
void *link;
unsigned int helper_stack_index;
public:
/*temp*/ int helper_n;
/*temp*/ void *helper_link;
/*temp*/ void *helper_link_1;
/*temp*/ void *otherhelper_link;
/*temp*/ void *otherhelper_link_1;
};
TopLevel top_level("top_level");
int gvar;
int i;
int j;

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ void *temp_link_1;
static const void *(lmap[]) = { &&LINK, &&PROCEED_THEN_ELSE, &&ENTER_helper, &&PROCEED_NEXT, &&PROCEED_NEXT_1, &&YIELD, &&YIELD_1, &&PROCEED_THEN_ELSE_1, &&LINK_1, &&ENTER_otherhelper };
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
 ::TopLevel::helper_link=(lmap[ ::TopLevel::T_STATE_LINK]);
 ::TopLevel::helper_n=temp_n;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_helper]);
goto *(state);
}
LINK:;
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD_1]);
goto *(state);
}
YIELD_1:;
 ::i=((1)+ ::i);
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
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
 ::j=(0);
{
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]));
goto *(state);
}
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::gvar=((1)+ ::gvar);
 ::j=((1)+ ::j);
{
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::TopLevel::otherhelper_link=(lmap[ ::TopLevel::T_STATE_LINK_1]);
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
