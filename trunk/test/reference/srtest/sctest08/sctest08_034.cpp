#include "isystemc.h"

class TopLevel;
int i;
int gvar;
int j;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
private:
unsigned int helper_stack_index;
public:
/*temp*/ void *otherhelper_link;
/*temp*/ int helper_n;
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link1;
private:
void *(link_stack[10U]);
public:
/*temp*/ void *helper_link1;
void T();
private:
int (n_stack[10U]);
void *link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ void *temp_link1;
/*temp*/ int temp_n;
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(4))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link1=(&&LINK);
goto ENTER_helper;
LINK:;
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
 ::i=( ::i+(1));
goto *(( ::i<(4)) ? (&&NEXT) : (&&PROCEED1));
PROCEED1:;
goto ELSE;
THEN:;
ELSE:;
cease(  ::gvar );
return ;
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
goto *((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&THEN1) : (&&PROCEED2));
PROCEED2:;
NEXT1:;
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&NEXT1) : (&&PROCEED3));
PROCEED3:;
goto ELSE1;
THEN1:;
ELSE1:;
 ::TopLevel::otherhelper_link1=(&&LINK1);
goto ENTER_otherhelper;
LINK1:;
temp_link1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
goto *(temp_link1);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link1;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
goto *(temp_link);
}