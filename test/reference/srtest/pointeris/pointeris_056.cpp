#include "isystemc.h"

void *(link_stack[10U]);
unsigned int main_stack_index = 0U;
int a = (44)+((99)*({ {
auto void *state;
wait(SC_ZERO_TIME);
state=(&&NEXT);
goto *(state);
NEXT:;
state=(&&CONTINUE);
goto *(state);
CONTINUE:;
state=((0) ? (&&NEXT) : (&&PROCEED));
goto *(state);
PROCEED:;
}
233; }));
void (main)();
void *main_link;
void *main_link1;
int main_return;

void (main)()
{
/*temp*/ void *temp_link;
auto void *state1;
/*temp*/ int result;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link1;
wait(SC_ZERO_TIME);
state1=(&&NEXT1);
goto *(state1);
NEXT1:;
state1=((0) ? (&&NEXT1) : (&&PROCEED1));
goto *(state1);
PROCEED1:;
result=(23);
 ::main_return=( ::a+((4)+((9)*result)));
temp_link=( ::link_stack[ ::main_stack_index]);
 ::main_stack_index--;
return ;
}
