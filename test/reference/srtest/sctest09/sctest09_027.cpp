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
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
{
 ::TopLevel::recurser_i=temp_i;
{
 ::TopLevel::recurser_link=(&&LINK);
goto ENTER_recurser;
}
}
LINK:;
}
});
cease(  ::TopLevel::x );
return ;
ENTER_recurser:;
{
{
/*temp*/ void *temp_link;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
{
if( !(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5)) )
goto THEN;
goto PROCEED;
PROCEED:;
{
({ /*temp*/ int temp_i1; temp_i1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])); {
{
 ::TopLevel::recurser_i=temp_i1;
{
 ::TopLevel::recurser_link=(&&LINK1);
goto ENTER_recurser;
}
}
LINK1:;
}
});
({ /*temp*/ int temp_i2; temp_i2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])); {
{
 ::TopLevel::recurser_i=temp_i2;
{
 ::TopLevel::recurser_link=(&&LINK2);
goto ENTER_recurser;
}
}
LINK2:;
}
});
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
