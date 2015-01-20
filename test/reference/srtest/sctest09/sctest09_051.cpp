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
enum TStates
{
T_STATE_LINK_THEN_ELSE = 4U,
T_STATE_LINK = 3U,
T_STATE_ENTER_recurser = 1U,
T_STATE_LINK_1 = 0U,
T_STATE_PROCEED = 2U,
};
void T();
private:
unsigned int (link_stack[10U]);
public:
/*temp*/ unsigned int recurser_link;
private:
int (i_stack[10U]);
public:
int x;
/*temp*/ int recurser_i;
/*temp*/ unsigned int recurser_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ int temp_i;
static const unsigned int (lmap[]) = { &&LINK, &&ENTER_recurser, &&PROCEED, &&LINK_1, &&LINK_THEN_ELSE };
/*temp*/ int temp_i_1;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i_2;
 ::TopLevel::x=(0);
temp_i_1=(1);
 ::TopLevel::recurser_link_1= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::recurser_i=temp_i_1;
wait(SC_ZERO_TIME);
{
state= ::TopLevel::T_STATE_ENTER_recurser;
goto *(lmap[state]);
}
LINK:;
cease(  ::TopLevel::x );
return ;
{
state= ::TopLevel::T_STATE_ENTER_recurser;
goto *(lmap[state]);
}
ENTER_recurser:;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link_1;
 ::TopLevel::x++;
{
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
}
PROCEED:;
temp_i_2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link_1= ::TopLevel::T_STATE_LINK;
 ::TopLevel::recurser_i=temp_i_2;
{
state= ::TopLevel::T_STATE_ENTER_recurser;
goto *(lmap[state]);
}
LINK_1:;
temp_i=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link_1= ::TopLevel::T_STATE_LINK_THEN_ELSE;
 ::TopLevel::recurser_i=temp_i;
{
state= ::TopLevel::T_STATE_ENTER_recurser;
goto *(lmap[state]);
}
LINK_THEN_ELSE:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
{
state=temp_link;
goto *(lmap[state]);
}
}
