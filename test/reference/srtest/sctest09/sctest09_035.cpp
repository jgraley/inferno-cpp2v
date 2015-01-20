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
void *(link_stack[10U]);
public:
void T();
/*temp*/ void *recurser_link;
int x;
/*temp*/ int recurser_i;
private:
unsigned int recurser_stack_index;
public:
/*temp*/ void *recurser_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_i;
/*temp*/ void *temp_link;
/*temp*/ int temp_i_1;
/*temp*/ int temp_i_2;
 ::TopLevel::x=(0);
temp_i=(1);
 ::TopLevel::recurser_link_1=(&&LINK);
 ::TopLevel::recurser_i=temp_i;
goto ENTER_recurser;
LINK:;
cease(  ::TopLevel::x );
return ;
ENTER_recurser:;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link_1;
 ::TopLevel::x++;
goto *((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
temp_i_1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link_1=(&&LINK_1);
 ::TopLevel::recurser_i=temp_i_1;
goto ENTER_recurser;
LINK_1:;
temp_i_2=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link_1=(&&LINK_2);
 ::TopLevel::recurser_i=temp_i_2;
goto ENTER_recurser;
LINK_2:;
goto THEN_ELSE;
THEN_ELSE:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
goto *(temp_link);
}
