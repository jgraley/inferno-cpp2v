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
private:
void *(link_stack[10U]);
public:
/*temp*/ void *helper_link;
/*temp*/ int helper_n;
private:
unsigned int helper_stack_index;
void *link;
public:
/*temp*/ void *otherhelper_link;
/*temp*/ void *helper_link_1;
/*temp*/ void *otherhelper_link_1;
private:
int (n_stack[10U]);
public:
void T();
};
TopLevel top_level("top_level");
int j;
int i;

void TopLevel::T()
{
/*temp*/ int temp_n;
/*temp*/ void *temp_link;
/*temp*/ void *temp_link_1;
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(4))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link=(&&LINK);
goto ENTER_helper;
LINK:;
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
 ::i=((1)+ ::i);
goto *(( ::i<(4)) ? (&&NEXT) : (&&PROCEED_1));
PROCEED_1:;
goto ELSE;
THEN:;
ELSE:;
cease(  ::gvar );
return ;
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
goto *((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&THEN_1) : (&&PROCEED_2));
PROCEED_2:;
NEXT_1:;
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
 ::j=((1)+ ::j);
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&NEXT_1) : (&&PROCEED_3));
PROCEED_3:;
goto ELSE_1;
THEN_1:;
ELSE_1:;
 ::TopLevel::otherhelper_link=(&&LINK_1);
goto ENTER_otherhelper;
LINK_1:;
temp_link_1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
goto *(temp_link_1);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
goto *(temp_link);
}
