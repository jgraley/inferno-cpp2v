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
int (n_stack[10U]);
public:
/*temp*/ void *otherhelper_link;
private:
unsigned int helper_stack_index;
public:
/*temp*/ void *helper_link;
private:
void *(link_stack[10U]);
public:
/*temp*/ void *helper_link_1;
/*temp*/ int helper_n;
private:
void *link;
public:
/*temp*/ void *otherhelper_link_1;
void T();
};
TopLevel top_level("top_level");
int j;
int i;
int gvar;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
goto *((!( ::i<(4))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
{
NEXT:;
{
{
 ::gvar=( ::gvar+ ::i);
{
/*temp*/ int temp_n;
temp_n=(3);
{
{
 ::TopLevel::helper_n=temp_n;
{
 ::TopLevel::helper_link_1=(&&LINK);
goto ENTER_helper;
}
}
LINK:;
}
}
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
}
CONTINUE:;
 ::i=((1)+ ::i);
}
CONTINUE_1:;
goto *(( ::i<(4)) ? (&&NEXT) : (&&PROCEED_1));
PROCEED_1:;
}
goto ELSE;
THEN:;
;
ELSE:;
}
}
cease(  ::gvar );
return ;
ENTER_helper:;
{
{
/*temp*/ void *temp_link;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link_1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
{
 ::j=(0);
{
goto *((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&THEN_1) : (&&PROCEED_2));
PROCEED_2:;
{
NEXT_1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
CONTINUE_2:;
 ::j=( ::j+(1));
}
CONTINUE_3:;
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&NEXT_1) : (&&PROCEED_3));
PROCEED_3:;
}
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
}
{
{
 ::TopLevel::otherhelper_link=(&&LINK_1);
goto ENTER_otherhelper;
}
LINK_1:;
}
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
{
 ::TopLevel::helper_stack_index--;
goto *(temp_link);
}
}
}
}
ENTER_otherhelper:;
{
/*temp*/ void *temp_link_1;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link_1= ::TopLevel::link;
goto *(temp_link_1);
}
}
}
