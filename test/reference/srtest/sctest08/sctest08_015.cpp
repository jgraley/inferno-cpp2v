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
void (helper)();
private:
unsigned int helper_stack_index;
void *link;
void *(link_stack[10U]);
int (n_stack[10U]);
public:
void (otherhelper)();
/*temp*/ void *helper_link1;
/*temp*/ void *helper_link;
/*temp*/ int helper_n;
/*temp*/ void *otherhelper_link;
/*temp*/ void *otherhelper_link1;
};
int gvar;
int i;
int j;
TopLevel top_level("top_level");

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
/*temp*/ void *temp_link1;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
for(  ::j=(0);  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]);  ::j=((1)+ ::j) )
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
{
{
 ::TopLevel::otherhelper_link1=(&&LINK1);
 ::TopLevel::otherhelper();
}
LINK1:;
}
{
temp_link1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
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
 ::TopLevel::link= ::TopLevel::otherhelper_link1;
 ::gvar=( ::gvar-(1));
{
temp_link= ::TopLevel::link;
return ;
}
}
