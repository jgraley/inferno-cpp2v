#include "isystemc.h"

unsigned int main_stack_index = 0U;
void *(link_stack[10U]);
void (main)();
int a = ((99)*({ {
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
233; }))+(44);
int main_return;
void *main_link;

void (main)()
{
/*temp*/ void *temp_link;
auto void *state;
/*temp*/ int result;
/*temp*/ bool enabled = true;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link;
wait(SC_ZERO_TIME);
state=(&&NEXT);
goto *(state);
NEXT:;
state=((0) ? (&&NEXT) : (&&PROCEED));
goto *(state);
PROCEED:;
result=(23);
 ::main_return=((((9)*result)+(4))+ ::a);
temp_link=( ::link_stack[ ::main_stack_index]);
 ::main_stack_index--;
enabled=(false);
}
