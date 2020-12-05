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
enum TStates
{
T_STATE_ENTER_recurser = 1U,
T_STATE_LINK1 = 0U,
T_STATE_LINK = 3U,
T_STATE_LINK_THEN_ELSE = 4U,
T_STATE_PROCEED = 2U,
};
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
/*temp*/ void *temp_link;
static const void *(lmap[]) = { &&LINK, &&ENTER_recurser, &&PROCEED, &&LINK1, &&LINK_THEN_ELSE };
auto void *state;
/*temp*/ int temp_i;
/*temp*/ int temp_i2;
/*temp*/ int temp_i1;
 ::TopLevel::x=(0);
temp_i=(1);
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser_link=(lmap[ ::TopLevel::T_STATE_LINK1]);
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_recurser]);
goto *(state);
}
LINK:;
cease(  ::TopLevel::x );
return ;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_recurser]);
goto *(state);
}
ENTER_recurser:;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
{
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ? (lmap[ ::TopLevel::T_STATE_LINK_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED:;
temp_i2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_i=temp_i2;
 ::TopLevel::recurser_link=(lmap[ ::TopLevel::T_STATE_LINK]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_recurser]);
goto *(state);
}
LINK1:;
temp_i1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_i=temp_i1;
 ::TopLevel::recurser_link=(lmap[ ::TopLevel::T_STATE_LINK_THEN_ELSE]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_recurser]);
goto *(state);
}
LINK_THEN_ELSE:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
{
state=temp_link;
goto *(state);
}
}
