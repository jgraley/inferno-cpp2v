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
/*temp*/ void *recurser_link;
private:
unsigned int recurser_stack_index;
int (i_stack[10U]);
void *(link_stack[10U]);
public:
int x;
void T();
/*temp*/ void *recurser_link_1;
/*temp*/ int recurser_i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
{
 ::TopLevel::recurser_link=(&&LINK);
{
 ::TopLevel::recurser_i=temp_i;
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
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
 ::TopLevel::x++;
{
if( !(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5)) )
goto THEN;
goto PROCEED;
PROCEED:;
{
({ /*temp*/ int temp_i_1; temp_i_1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])); {
{
 ::TopLevel::recurser_link=(&&LINK_1);
{
 ::TopLevel::recurser_i=temp_i_1;
goto ENTER_recurser;
}
}
LINK_1:;
}
});
({ /*temp*/ int temp_i_2; temp_i_2=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1)); {
{
 ::TopLevel::recurser_link=(&&LINK_2);
{
 ::TopLevel::recurser_i=temp_i_2;
goto ENTER_recurser;
}
}
LINK_2:;
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
