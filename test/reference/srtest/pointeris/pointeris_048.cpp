#include "isystemc.h"

unsigned int main_stack_index = 0U;
void *(link_stack[10U]);
void (main)();
int a = ((99)*({ {
wait(SC_ZERO_TIME);
goto NEXT;
NEXT:;
goto CONTINUE;
CONTINUE:;
goto *((0) ? (&&NEXT) : (&&PROCEED));
PROCEED:;
}
233; }))+(44);
int main_return;
void *main_link;
void *main_link1;

void (main)()
{
/*temp*/ void *temp_link;
/*temp*/ int result;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link1;
wait(SC_ZERO_TIME);
goto NEXT;
NEXT:;
goto *((0) ? (&&NEXT) : (&&PROCEED));
PROCEED:;
result=(23);
 ::main_return=((((9)*result)+(4))+ ::a);
temp_link=( ::link_stack[ ::main_stack_index]);
 ::main_stack_index--;
return ;
}
