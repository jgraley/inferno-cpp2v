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
void T();
void (recurser)();
private:
unsigned int recurser_stack_index;
void *(link_stack[10U]);
int (i_stack[10U]);
public:
/*temp*/ void *recurser_link;
/*temp*/ int recurser_i;
/*temp*/ void *recurser_link1;
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
 ::TopLevel::recurser();
}
}
LINK:;
}
});
cease(  ::TopLevel::x );
return ;
}

void (TopLevel::recurser)()
{
{
/*temp*/ void *temp_link;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link1;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
if( ( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5) )
{
({ /*temp*/ int temp_i; temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1)); {
{
 ::TopLevel::recurser_i=temp_i;
{
 ::TopLevel::recurser_link1=(&&LINK);
 ::TopLevel::recurser();
}
}
LINK:;
}
});
({ /*temp*/ int temp_i1; temp_i1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1)); {
{
 ::TopLevel::recurser_i=temp_i1;
{
 ::TopLevel::recurser_link1=(&&LINK1);
 ::TopLevel::recurser();
}
}
LINK1:;
}
});
}
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
{
 ::TopLevel::recurser_stack_index--;
return ;
}
}
}
}
