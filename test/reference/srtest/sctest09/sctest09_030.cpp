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
unsigned int recurser_stack_index;
public:
void T();
/*temp*/ void *recurser_link;
/*temp*/ int recurser_i;
/*temp*/ void *recurser_link1;
private:
void *(link_stack[10U]);
public:
int x;
private:
int (i_stack[10U]);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
{
/*temp*/ int temp_i;
temp_i=(1);
{
{
 ::TopLevel::recurser_link1=(&&LINK);
{
 ::TopLevel::recurser_i=temp_i;
goto ENTER_recurser;
}
}
LINK:;
}
}
cease(  ::TopLevel::x );
return ;
ENTER_recurser:;
{
{
/*temp*/ void *temp_link;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link1;
 ::TopLevel::x++;
{
goto *((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
{
{
/*temp*/ int temp_i1;
temp_i1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
{
{
 ::TopLevel::recurser_link1=(&&LINK1);
{
 ::TopLevel::recurser_i=temp_i1;
goto ENTER_recurser;
}
}
LINK1:;
}
}
{
/*temp*/ int temp_i2;
temp_i2=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
{
{
 ::TopLevel::recurser_link1=(&&LINK2);
{
 ::TopLevel::recurser_i=temp_i2;
goto ENTER_recurser;
}
}
LINK2:;
}
}
}
goto ELSE;
THEN:;
;
ELSE:;
}
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
{
 ::TopLevel::recurser_stack_index--;
goto *(temp_link);
}
}
}
}
}
