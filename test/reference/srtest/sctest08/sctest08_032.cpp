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
private:
void *(link_stack[10U]);
unsigned int helper_stack_index;
public:
void T();
/*temp*/ void *otherhelper_link;
private:
void *link;
public:
/*temp*/ void *helper_link;
/*temp*/ void *helper_link_1;
/*temp*/ int helper_n;
/*temp*/ void *otherhelper_link_1;
private:
int (n_stack[10U]);
};
TopLevel top_level("top_level");
int j;
int gvar;
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
 ::TopLevel::helper_link_1=(&&LINK);
goto ENTER_helper;
LINK:;
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
CONTINUE:;
 ::i=( ::i+(1));
CONTINUE_1:;
goto *(( ::i<(4)) ? (&&NEXT) : (&&PROCEED_1));
PROCEED_1:;
goto ELSE;
THEN:;
;
ELSE:;
cease(  ::gvar );
return ;
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link_1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
goto *((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&THEN_1) : (&&PROCEED_2));
PROCEED_2:;
NEXT_1:;
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
CONTINUE_2:;
 ::j=( ::j+(1));
CONTINUE_3:;
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&NEXT_1) : (&&PROCEED_3));
PROCEED_3:;
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
 ::TopLevel::otherhelper_link_1=(&&LINK_1);
goto ENTER_otherhelper;
LINK_1:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
goto *(temp_link);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link_1;
 ::gvar=( ::gvar-(1));
temp_link_1= ::TopLevel::link;
goto *(temp_link_1);
}
