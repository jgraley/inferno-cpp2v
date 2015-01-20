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
void *(link_stack[10U]);
public:
void T();
enum TStates
{
T_STATE_ENTER_recurser = 1U,
T_STATE_LINK = 3U,
T_STATE_LINK_THEN_ELSE = 4U,
T_STATE_PROCEED = 2U,
T_STATE_LINK_1 = 0U,
};
/*temp*/ void *recurser_link;
/*temp*/ int recurser_i;
private:
unsigned int recurser_stack_index;
public:
int x;
private:
int (i_stack[10U]);
public:
/*temp*/ void *recurser_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ int temp_i;
static const void *(lmap[]) = { &&LINK, &&ENTER_recurser, &&PROCEED, &&LINK_1, &&LINK_THEN_ELSE };
/*temp*/ int temp_i_1;
/*temp*/ int temp_i_2;
auto void *state;
 ::TopLevel::x=(0);
temp_i_2=(1);
 ::TopLevel::recurser_link_1=(lmap[ ::TopLevel::T_STATE_LINK_1]);
 ::TopLevel::recurser_i=temp_i_2;
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
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link_1;
 ::TopLevel::x++;
{
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ? (lmap[ ::TopLevel::T_STATE_LINK_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED:;
temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link_1=(lmap[ ::TopLevel::T_STATE_LINK]);
 ::TopLevel::recurser_i=temp_i;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_recurser]);
goto *(state);
}
LINK_1:;
temp_i_1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link_1=(lmap[ ::TopLevel::T_STATE_LINK_THEN_ELSE]);
 ::TopLevel::recurser_i=temp_i_1;
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
