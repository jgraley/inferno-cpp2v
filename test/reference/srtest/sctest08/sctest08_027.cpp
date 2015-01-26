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
/*temp*/ int helper_n;
private:
void *(link_stack[10U]);
public:
/*temp*/ void *otherhelper_link;
void T();
/*temp*/ void *otherhelper_link1;
private:
void *link;
public:
/*temp*/ void *helper_link;
/*temp*/ void *helper_link1;
private:
int (n_stack[10U]);
unsigned int helper_stack_index;
};
int gvar;
TopLevel top_level("top_level");
int i;
int j;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(4)) )
goto THEN;
goto PROCEED;
PROCEED:;
{
NEXT:;
{
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3); {
{
 ::TopLevel::helper_n=temp_n;
{
 ::TopLevel::helper_link1=(&&LINK);
goto ENTER_helper;
}
}
LINK:;
}
});
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
}
CONTINUE:;
 ::i=( ::i+(1));
}
CONTINUE1:;
if(  ::i<(4) )
goto NEXT;
goto PROCEED1;
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
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
{
 ::j=(0);
{
if( !( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) )
goto THEN1;
goto PROCEED2;
PROCEED2:;
{
NEXT1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
}
CONTINUE2:;
 ::j=((1)+ ::j);
}
CONTINUE3:;
if(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) )
goto NEXT1;
goto PROCEED3;
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
