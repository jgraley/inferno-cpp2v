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
void *(link_stack[10U]);
public:
void T();
/*temp*/ void *recurser_link;
/*temp*/ int recurser_i;
int x;
private:
unsigned int recurser_stack_index;
int (i_stack[10U]);
public:
/*temp*/ void *recurser_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_i;
/*temp*/ void *temp_link;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
 ::TopLevel::x=(0);
temp_i2=(1);
 ::TopLevel::recurser_link1=(&&LINK);
 ::TopLevel::recurser_i=temp_i2;
goto ENTER_recurser;
LINK:;
cease(  ::TopLevel::x );
return ;
goto ENTER_recurser;
ENTER_recurser:;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link1;
 ::TopLevel::x++;
goto *((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ? (&&LINK_THEN_ELSE) : (&&PROCEED));
PROCEED:;
temp_i1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link1=(&&LINK1);
 ::TopLevel::recurser_i=temp_i1;
goto ENTER_recurser;
LINK1:;
temp_i=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link1=(&&LINK_THEN_ELSE);
 ::TopLevel::recurser_i=temp_i;
goto ENTER_recurser;
LINK_THEN_ELSE:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
goto *(temp_link);
}