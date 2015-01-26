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
/*temp*/ void *recurser_link;
/*temp*/ void *recurser_link1;
/*temp*/ int recurser_i;
int x;
private:
void *(link_stack[10U]);
unsigned int recurser_stack_index;
int (i_stack[10U]);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ void *temp_link;
/*temp*/ int temp_i2;
 ::TopLevel::x=(0);
temp_i=(1);
 ::TopLevel::recurser_link=(&&LINK);
 ::TopLevel::recurser_i=temp_i;
goto ENTER_recurser;
LINK:;
cease(  ::TopLevel::x );
return ;
ENTER_recurser:;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
 ::TopLevel::x++;
goto *((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ? (&&LINK_THEN_ELSE) : (&&PROCEED));
PROCEED:;
temp_i1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link=(&&LINK1);
 ::TopLevel::recurser_i=temp_i1;
goto ENTER_recurser;
LINK1:;
temp_i2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link=(&&LINK_THEN_ELSE);
 ::TopLevel::recurser_i=temp_i2;
goto ENTER_recurser;
goto LINK_THEN_ELSE;
LINK_THEN_ELSE:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
goto *(temp_link);
}
