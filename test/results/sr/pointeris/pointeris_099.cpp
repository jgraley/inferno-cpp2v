#include "isystemc.h"

void *(link_stack[10]);
unsigned int main_stack_index = 0;
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
int main_return;
void *main_link;

void (main)()
{
/*temp*/ void *temp_link;
auto void *state_1;
/*temp*/ int result;
/*temp*/ bool enabled = true;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link;
wait(SC_ZERO_TIME);
state_1=(&&NEXT_1);
goto *(state_1);
NEXT_1:;
state_1=((0) ? (&&NEXT_1) : (&&PROCEED_1));
goto *(state_1);
PROCEED_1:;
result=(23);
 ::main_return=( ::a+((4)+((9)*result)));
temp_link=( ::link_stack[ ::main_stack_index]);
 ::main_stack_index--;
enabled=(false);
}
