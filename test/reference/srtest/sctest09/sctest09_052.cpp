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
/*temp*/ unsigned int recurser_link;
/*temp*/ int recurser_i;
/*temp*/ unsigned int recurser_link1;
private:
unsigned int recurser_stack_index;
int (i_stack[10U]);
unsigned int (link_stack[10U]);
public:
enum TStates
{
T_STATE_LINK = 0U,
T_STATE_ENTER_recurser = 1U,
T_STATE_PROCEED = 2U,
T_STATE_LINK1 = 3U,
T_STATE_LINK_THEN_ELSE = 4U,
};
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i2;
static const unsigned int (lmap[]) = { &&LINK, &&ENTER_recurser, &&PROCEED, &&LINK1, &&LINK_THEN_ELSE };
 ::TopLevel::x=(0);
temp_i2=(1);
 ::TopLevel::recurser_i=temp_i2;
 ::TopLevel::recurser_link1= ::TopLevel::T_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_recurser;
goto *(lmap[state]);
LINK:;
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_recurser;
goto *(lmap[state]);
ENTER_recurser:;
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link1;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
PROCEED:;
temp_i1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_i=temp_i1;
 ::TopLevel::recurser_link1= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_recurser;
goto *(lmap[state]);
LINK1:;
temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser_link1= ::TopLevel::T_STATE_LINK_THEN_ELSE;
state= ::TopLevel::T_STATE_ENTER_recurser;
goto *(lmap[state]);
LINK_THEN_ELSE:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
state=temp_link;
goto *(lmap[state]);
}