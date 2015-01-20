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
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
private:
unsigned int helper_stack_index;
public:
/*temp*/ int helper_n;
void T();
private:
void *(link_stack[10U]);
public:
/*temp*/ void *helper_link_1;
private:
void *link;
public:
void (helper)();
void (otherhelper)();
/*temp*/ void *otherhelper_link_1;
private:
int (n_stack[10U]);
};
TopLevel top_level("top_level");
int i;
int j;

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i=((1)+ ::i) )
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3); {
{
 ::TopLevel::helper_n=temp_n;
{
 ::TopLevel::helper_link=(&&LINK);
 ::TopLevel::helper();
}
}
LINK:;
}
});
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
return ;
}

void (TopLevel::helper)()
{
{
/*temp*/ void *temp_link;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
for(  ::j=(0);  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]);  ::j=( ::j+(1)) )
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
}
{
{
 ::TopLevel::otherhelper_link=(&&LINK);
 ::TopLevel::otherhelper();
}
LINK:;
}
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
{
 ::TopLevel::helper_stack_index--;
return ;
}
}
}
}

void (TopLevel::otherhelper)()
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link= ::TopLevel::link;
return ;
}
}
