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
void T();
/*temp*/ unsigned int recurser_link;
/*temp*/ unsigned int recurser_link1;
/*temp*/ int recurser_i;
private:
int (i_stack[10U]);
unsigned int (link_stack[10U]);
public:
enum TStates
{
T_STATE_LINK_THEN_ELSE = 4U,
T_STATE_PROCEED = 2U,
T_STATE_LINK = 0U,
T_STATE_ENTER_recurser = 1U,
T_STATE_LINK1 = 3U,
};
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
static const unsigned int (lmap[]) = { &&LINK1, &&ENTER_recurser, &&PROCEED, &&LINK, &&LINK_THEN_ELSE };
auto unsigned int state;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i2;
 ::TopLevel::x=(0);
temp_i2=(1);
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::recurser_i=temp_i2;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_recurser;
LINK_THEN_ELSE:;
LINK:;
PROCEED:;
ENTER_recurser:;
LINK1:;
if( state== ::TopLevel::T_STATE_LINK )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::T_STATE_ENTER_recurser==state )
{
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
 ::TopLevel::x++;
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if( state== ::TopLevel::T_STATE_PROCEED )
{
temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::recurser_i=temp_i;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
temp_i1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_THEN_ELSE;
 ::TopLevel::recurser_i=temp_i1;
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
