#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
recurser_stack_index(0U)
{
SC_THREAD(T);
}
private:
int (i_stack[10U]);
public:
/*temp*/ int recurser_i;
void T();
private:
unsigned int recurser_stack_index;
public:
int x;
/*temp*/ void *recurser_link;
/*temp*/ void *recurser_link_1;
private:
void *(link_stack[10U]);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ int temp_i;
auto void *state;
/*temp*/ int temp_i_1;
/*temp*/ int temp_i_2;
 ::TopLevel::x=(0);
temp_i_1=(1);
 ::TopLevel::recurser_link_1=(&&LINK);
 ::TopLevel::recurser_i=temp_i_1;
wait(SC_ZERO_TIME);
{
state=(&&ENTER_recurser);
goto *(state);
}
LINK:;
cease(  ::TopLevel::x );
return ;
{
state=(&&ENTER_recurser);
goto *(state);
}
ENTER_recurser:;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link_1;
 ::TopLevel::x++;
{
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ? (&&LINK_THEN_ELSE) : (&&PROCEED));
goto *(state);
}
PROCEED:;
temp_i_2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link_1=(&&LINK_1);
 ::TopLevel::recurser_i=temp_i_2;
{
state=(&&ENTER_recurser);
goto *(state);
}
LINK_1:;
temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link_1=(&&LINK_THEN_ELSE);
 ::TopLevel::recurser_i=temp_i;
{
state=(&&ENTER_recurser);
goto *(state);
}
LINK_THEN_ELSE:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
{
state=temp_link;
goto *(state);
}
}
