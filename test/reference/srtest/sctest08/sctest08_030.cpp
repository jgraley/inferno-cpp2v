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
void T();
private:
unsigned int helper_stack_index;
void *link;
void *(link_stack[10U]);
int (n_stack[10U]);
public:
/*temp*/ void *helper_link1;
/*temp*/ void *helper_link;
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
 ::TopLevel::helper_link=(&&LINK);
goto ENTER_helper;
}
}
LINK:;
}
}
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
CONTINUE:;
 ::i=((1)+ ::i);
}
CONTINUE1:;
goto *(( ::i<(4)) ? (&&NEXT) : (&&PROCEED1));
PROCEED1:;
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
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
{
 ::j=(0);
{
goto *((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&THEN1) : (&&PROCEED2));
PROCEED2:;
{
NEXT1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
CONTINUE2:;
 ::j=((1)+ ::j);
}
CONTINUE3:;
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&NEXT1) : (&&PROCEED3));
PROCEED3:;
}
goto ELSE1;
THEN1:;
;
ELSE1:;
}
}
{
{
 ::TopLevel::otherhelper_link=(&&LINK1);
goto ENTER_otherhelper;
}
LINK1:;
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
/*temp*/ void *temp_link1;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link1= ::TopLevel::link;
goto *(temp_link1);
}
}
}
