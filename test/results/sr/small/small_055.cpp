void *(link_stack[10]);
unsigned int main_stack_index = 0;
void (main)();
int main_return;
void *main_link;
void *main_link_1;

void (main)()
{
/*temp*/ void *temp_link;
 ::main_stack_index++;
( ::link_stack[ ::main_stack_index])= ::main_link;
 ::main_return=(9);
temp_link=( ::link_stack[ ::main_stack_index]);
 ::main_stack_index--;
return ;
}
