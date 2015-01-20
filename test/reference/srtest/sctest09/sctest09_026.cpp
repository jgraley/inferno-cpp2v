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
/*temp*/ int recurser_i;
int x;
private:
void *(link_stack[10U]);
public:
/*temp*/ void *recurser_link_1;
private:
unsigned int recurser_stack_index;
int (i_stack[10U]);
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
{
 ::TopLevel::recurser_link_1=(&&LINK);
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
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link_1;
 ::TopLevel::x++;
{
if( !(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5)) )
goto THEN;
{
({ /*temp*/ int temp_i_1; temp_i_1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1)); {
{
 ::TopLevel::recurser_link_1=(&&LINK_1);
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
 ::TopLevel::recurser_link_1=(&&LINK_2);
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
