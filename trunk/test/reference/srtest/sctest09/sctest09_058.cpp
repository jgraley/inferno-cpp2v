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
T_STATE_LINK = 0U,
T_STATE_LINK1 = 3U,
T_STATE_PROCEED = 2U,
T_STATE_ENTER_recurser = 1U,
T_STATE_LINK_THEN_ELSE = 4U,
};
private:
unsigned int recurser_stack_index;
public:
int x;
/*temp*/ unsigned int recurser_link;
/*temp*/ int recurser_i;
private:
unsigned int (link_stack[10U]);
public:
/*temp*/ unsigned int recurser_link1;
void T();
private:
int (i_stack[10U]);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ int temp_i;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
static const unsigned int (lmap[]) = { &&LINK, &&ENTER_recurser, &&PROCEED, &&LINK1, &&LINK_THEN_ELSE };
 ::TopLevel::x=(0);
temp_i=(1);
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::recurser_i=temp_i;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_recurser;
LINK:;
if( state== ::TopLevel::T_STATE_LINK )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
ENTER_recurser:;
if( state== ::TopLevel::T_STATE_ENTER_recurser )
{
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
 ::TopLevel::x++;
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
PROCEED:;
if( state== ::TopLevel::T_STATE_PROCEED )
{
temp_i1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::recurser_i=temp_i1;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
LINK1:;
if( state== ::TopLevel::T_STATE_LINK1 )
{
temp_i2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_THEN_ELSE;
 ::TopLevel::recurser_i=temp_i2;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
LINK_THEN_ELSE:;
if( state== ::TopLevel::T_STATE_LINK_THEN_ELSE )
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
state=temp_link;
}
goto *(lmap[state]);
}
