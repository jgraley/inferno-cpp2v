#include "isystemc.h"

class TopLevel;
int i;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
private:
int (n_stack[10U]);
public:
void T();
private:
void *link;
public:
/*temp*/ void *helper_link;
private:
unsigned int helper_stack_index;
public:
/*temp*/ void *otherhelper_link;
/*temp*/ void *helper_link_1;
private:
void *(link_stack[10U]);
public:
/*temp*/ void *otherhelper_link_1;
/*temp*/ int helper_n;
};
TopLevel top_level("top_level");
int gvar;
int j;

void TopLevel::T()
{
/*temp*/ int temp_n;
/*temp*/ void *temp_link;
/*temp*/ void *temp_link_1;
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(4))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link=(&&LINK);
goto ENTER_helper;
LINK:;
 ::gvar=( ::gvar*(2));
{
wait(SC_ZERO_TIME);
goto YIELD;
YIELD:;
}
 ::i=( ::i+(1));
goto *(( ::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
goto *((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&PROCEED_THEN_ELSE_1) : (&&PROCEED_NEXT_1));
PROCEED_NEXT_1:;
{
wait(SC_ZERO_TIME);
goto YIELD_1;
YIELD_1:;
}
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&PROCEED_NEXT_1) : (&&PROCEED_THEN_ELSE_1));
PROCEED_THEN_ELSE_1:;
 ::TopLevel::otherhelper_link_1=(&&LINK_1);
goto ENTER_otherhelper;
LINK_1:;
temp_link_1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
goto *(temp_link_1);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link_1;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
goto *(temp_link);
}
