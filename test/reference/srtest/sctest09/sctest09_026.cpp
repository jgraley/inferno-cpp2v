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
int x;
/*temp*/ void *recurser_link;
/*temp*/ int recurser_i;
/*temp*/ void *recurser_link1;
private:
unsigned int recurser_stack_index;
int (i_stack[10U]);
void *(link_stack[10U]);
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
{
 ::TopLevel::recurser_i=temp_i;
{
 ::TopLevel::recurser_link1=(&&LINK);
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
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link1;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
{
if( !(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5)) )
goto THEN;
{
({ /*temp*/ int temp_i1; temp_i1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1)); {
{
 ::TopLevel::recurser_i=temp_i1;
{
 ::TopLevel::recurser_link1=(&&LINK1);
goto ENTER_recurser;
}
}
LINK1:;
}
});
({ /*temp*/ int temp_i2; temp_i2=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1)); {
{
 ::TopLevel::recurser_i=temp_i2;
{
 ::TopLevel::recurser_link1=(&&LINK2);
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