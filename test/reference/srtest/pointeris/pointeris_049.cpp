#include "isystemc.h"

void *(link_stack[10U]);
unsigned int main_stack_index = 0U;
int a = ((99)*({ {
auto void *state;
wait(SC_ZERO_TIME);
{
state=(&&NEXT);
goto *(state);
}
NEXT:;
{
state=(&&CONTINUE);
goto *(state);
}
CONTINUE:;
{
state=((0) ? (&&NEXT) : (&&PROCEED));
goto *(state);
}
PROCEED:;
}
233; }))+(44);
void (main)();
void *main_link;
void *main_link1;
int main_return;

void (main)()
{
/*temp*/ void *temp_link;
auto void *state;
/*temp*/ int result;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link1;
wait(SC_ZERO_TIME);
{
state=(&&NEXT);
goto *(state);
}
NEXT:;
{
state=((0) ? (&&NEXT) : (&&PROCEED));
goto *(state);
}
PROCEED:;
result=(23);
 ::main_return=((((9)*result)+(4))+ ::a);
temp_link=( ::link_stack[ ::main_stack_index]);
 ::main_stack_index--;
return ;
}
