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
unsigned int (link_stack[10U]);
unsigned int recurser_stack_index;
unsigned int state;
public:
int x;
/*temp*/ int recurser_i;
/*temp*/ unsigned int recurser_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i;
/*temp*/ int temp_i2;
/*temp*/ int temp_i1;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_i=(1);
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
cease(  ::TopLevel::x );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_recurser )
{
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
 ::TopLevel::state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
temp_i2=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_i=temp_i2;
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
temp_i1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_i=temp_i1;
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_THEN_ELSE;
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
