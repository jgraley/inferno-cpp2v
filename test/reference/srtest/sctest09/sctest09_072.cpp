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
T_STATE_LINK = 3U,
T_STATE_LINK_THEN_ELSE = 4U,
T_STATE_PROCEED = 2U,
T_STATE_ENTER_recurser = 1U,
T_STATE_LINK_1 = 0U,
};
void T();
/*temp*/ unsigned int recurser_link;
private:
unsigned int recurser_stack_index;
int (i_stack[10U]);
unsigned int state;
public:
/*temp*/ int recurser_i;
int x;
private:
unsigned int (link_stack[10U]);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i;
/*temp*/ int temp_i_1;
/*temp*/ int temp_i_2;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_i_2=(1);
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::recurser_i=temp_i_2;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK_1 )
{
cease(  ::TopLevel::x );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_recurser )
{
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
 ::TopLevel::x++;
 ::TopLevel::state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
temp_i=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
temp_i_1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_THEN_ELSE;
 ::TopLevel::recurser_i=temp_i_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK_THEN_ELSE )
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
 ::TopLevel::state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
