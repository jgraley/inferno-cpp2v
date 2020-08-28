int a = ((99)*({ {
NEXT:;
{
}
CONTINUE:;
goto *((0) ? (&&NEXT) : (&&PROCEED));
PROCEED:;
}
233; }))+(44);
int main_return;
void *main_link;
void *main_link1;
unsigned int main_stack_index = 0U;
void (main)();
void *(link_stack[10U]);

void (main)()
{
{
/*temp*/ void *temp_link;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link1;
{
 ::main_return=((((9)*({ {
NEXT:;
{
}
CONTINUE:;
goto *((0) ? (&&NEXT) : (&&PROCEED));
PROCEED:;
}
23; }))+(4))+ ::a);
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
