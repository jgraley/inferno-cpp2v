void *(link_stack[10U]);
unsigned int main_stack_index = 0U;
int a = ((99)*({ {
NEXT:;
{
}
CONTINUE:;
goto *((0) ? (&&NEXT) : (&&PROCEED));
PROCEED:;
}
233; }))+(44);
void (main)();
void *main_link;
void *main_link1;
int main_return;

void (main)()
{
{
/*temp*/ void *temp_link;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link1;
{
{
/*temp*/ int result;
{
NEXT:;
{
}
CONTINUE:;
goto *((0) ? (&&NEXT) : (&&PROCEED));
PROCEED:;
}
result=(23);
 ::main_return=((((9)*result)+(4))+ ::a);
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
