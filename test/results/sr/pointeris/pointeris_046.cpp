void *(link_stack[10]);
unsigned int main_stack_index = 0;
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
void *main_link_1;

void (main)()
{
{
/*temp*/ void *temp_link;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link;
{
 ::main_return=( ::a+((4)+((9)*({ {
NEXT_1:;
{
}
CONTINUE_1:;
goto *((0) ? (&&NEXT_1) : (&&PROCEED_1));
PROCEED_1:;
}
23; }))));
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
