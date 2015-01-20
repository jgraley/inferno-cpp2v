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
/*temp*/ int recurser_i;
int x;
private:
int (i_stack[10U]);
public:
/*temp*/ void *recurser_link;
private:
unsigned int recurser_stack_index;
public:
/*temp*/ void *recurser_link_1;
void T();
private:
void *(link_stack[10U]);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_i;
/*temp*/ int temp_i_1;
/*temp*/ void *temp_link;
/*temp*/ int temp_i_2;
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
temp_i_1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link=(&&LINK_1);
 ::TopLevel::recurser_i=temp_i_1;
goto ENTER_recurser;
LINK_1:;
temp_i_2=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link=(&&LINK_THEN_ELSE);
 ::TopLevel::recurser_i=temp_i_2;
goto ENTER_recurser;
goto LINK_THEN_ELSE;
LINK_THEN_ELSE:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
goto *(temp_link);
}
