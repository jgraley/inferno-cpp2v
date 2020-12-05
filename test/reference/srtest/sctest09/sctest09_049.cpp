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
void T();
private:
int (i_stack[10U]);
void *(link_stack[10U]);
unsigned int recurser_stack_index;
public:
int x;
/*temp*/ int recurser_i;
/*temp*/ void *recurser_link1;
/*temp*/ void *recurser_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
auto void *state;
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
 ::TopLevel::x=(0);
temp_i=(1);
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser_link=(&&LINK);
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
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
{
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ? (&&LINK_THEN_ELSE) : (&&PROCEED));
goto *(state);
}
PROCEED:;
temp_i1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_i=temp_i1;
 ::TopLevel::recurser_link=(&&LINK1);
{
state=(&&ENTER_recurser);
goto *(state);
}
LINK1:;
temp_i2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_i=temp_i2;
 ::TopLevel::recurser_link=(&&LINK_THEN_ELSE);
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
