void *(link_stack[10U]);
unsigned int main_stack_index = 0U;
int a = (44)+((99)*({ {
NEXT:;
{
}
CONTINUE:;
goto *((0) ? (&&NEXT) : (&&PROCEED));
PROCEED:;
}
233; }));
void (main)();
int main_return;
void *main_link;
void *main_link1;

void (main)()
{
{
/*temp*/ void *temp_link;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link;
{
{
/*temp*/ int result;
{
NEXT1:;
{
}
CONTINUE1:;
goto *((0) ? (&&NEXT1) : (&&PROCEED1));
PROCEED1:;
}
result=(23);
 ::main_return=( ::a+((4)+((9)*result)));
}
{
temp_link=( ::link_stack[ ::main_stack_index]);
{
 ::main_stack_index--;
return ;
}
}
}
}
}
