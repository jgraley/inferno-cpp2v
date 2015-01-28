#include "isystemc.h"

class TopLevel;
int gvar;
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
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
/*temp*/ void *otherhelper_link1;
/*temp*/ int helper_n;
/*temp*/ void *helper_link1;
private:
void *link;
unsigned int helper_stack_index;
int (n_stack[10U]);
void *(link_stack[10U]);
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto void *state;
/*temp*/ void *temp_link;
/*temp*/ void *temp_link1;
/*temp*/ int temp_n;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(4))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
goto *(state);
}
PROCEED_NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link1=(&&LINK);
{
state=(&&ENTER_helper);
goto *(state);
}
LINK:;
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
{
state=(&&YIELD);
goto *(state);
}
YIELD:;
 ::i=( ::i+(1));
{
state=(( ::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
{
state=(&&ENTER_helper);
goto *(state);
}
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
{
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&PROCEED_THEN_ELSE1) : (&&PROCEED_NEXT1));
goto *(state);
}
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
{
state=(&&YIELD1);
goto *(state);
}
YIELD1:;
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
{
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&PROCEED_NEXT1) : (&&PROCEED_THEN_ELSE1));
goto *(state);
}
PROCEED_THEN_ELSE1:;
 ::TopLevel::otherhelper_link1=(&&LINK1);
{
state=(&&ENTER_otherhelper);
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
 ::TopLevel::link= ::TopLevel::otherhelper_link1;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
