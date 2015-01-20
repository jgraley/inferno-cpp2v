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
void (recurser)();
private:
int (i_stack[10U]);
public:
int x;
/*temp*/ int recurser_i;
/*temp*/ void *recurser_link;
private:
unsigned int recurser_stack_index;
public:
void T();
/*temp*/ void *recurser_link_1;
private:
void *(link_stack[10U]);
};
TopLevel top_level("top_level");

void (TopLevel::recurser)()
{
{
/*temp*/ void *temp_link;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link_1;
 ::TopLevel::x++;
if( ( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5) )
{
({ /*temp*/ int temp_i; temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1)); {
{
 ::TopLevel::recurser_link_1=(&&LINK);
{
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser();
}
}
LINK:;
}
});
({ /*temp*/ int temp_i_1; temp_i_1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1)); {
{
 ::TopLevel::recurser_link_1=(&&LINK_1);
{
 ::TopLevel::recurser_i=temp_i_1;
 ::TopLevel::recurser();
}
}
LINK_1:;
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

void TopLevel::T()
{
 ::TopLevel::x=(0);
({ /*temp*/ int temp_i; temp_i=(1); {
{
 ::TopLevel::recurser_link_1=(&&LINK);
{
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser();
}
}
LINK:;
}
});
cease(  ::TopLevel::x );
return ;
}
