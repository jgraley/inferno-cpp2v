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
unsigned int (link_stack[10U]);
int (i_stack[10U]);
public:
/*temp*/ unsigned int recurser_link;
int x;
enum TStates
{
T_STATE_ENTER_recurser = 1U,
T_STATE_LINK = 0U,
T_STATE_LINK1 = 3U,
T_STATE_LINK_THEN_ELSE = 4U,
T_STATE_PROCEED = 2U,
};
private:
unsigned int recurser_stack_index;
public:
/*temp*/ unsigned int recurser_link1;
/*temp*/ int recurser_i;
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i;
static const unsigned int (lmap[]) = { &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK, &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK, &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK, &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK, &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK };
auto unsigned int state;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
 ::TopLevel::x=(0);
temp_i1=(1);
 ::TopLevel::recurser_link1= ::TopLevel::T_STATE_LINK;
 ::TopLevel::recurser_i=temp_i1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_recurser;
LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK:;
if( state== ::TopLevel::T_STATE_LINK )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( state== ::TopLevel::T_STATE_ENTER_recurser )
{
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link1;
 ::TopLevel::x++;
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if( state== ::TopLevel::T_STATE_PROCEED )
{
temp_i2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link1= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::recurser_i=temp_i2;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link1= ::TopLevel::T_STATE_LINK_THEN_ELSE;
 ::TopLevel::recurser_i=temp_i;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( state== ::TopLevel::T_STATE_LINK_THEN_ELSE )
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
state=temp_link;
}
goto *(lmap[state]);
}
