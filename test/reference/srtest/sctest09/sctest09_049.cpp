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
int x;
private:
void *(link_stack[10U]);
public:
/*temp*/ int recurser_i;
/*temp*/ void *recurser_link;
private:
unsigned int recurser_stack_index;
public:
/*temp*/ void *recurser_link1;
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_i;
auto void *state;
/*temp*/ void *temp_link;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
 ::TopLevel::x=(0);
temp_i2=(1);
 ::TopLevel::recurser_link=(&&LINK);
 ::TopLevel::recurser_i=temp_i2;
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
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
 ::TopLevel::x++;
{
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ? (&&LINK_THEN_ELSE) : (&&PROCEED));
goto *(state);
}
PROCEED:;
temp_i1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link=(&&LINK1);
 ::TopLevel::recurser_i=temp_i1;
{
state=(&&ENTER_recurser);
goto *(state);
}
LINK1:;
temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link=(&&LINK_THEN_ELSE);
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
