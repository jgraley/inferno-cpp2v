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
/*temp*/ int temp_i;
/*temp*/ unsigned int temp_link;
void T();
/*temp*/ int temp_i1;
enum TStates
{
T_STATE_PROCEED = 2U,
T_STATE_LINK_THEN_ELSE = 4U,
T_STATE_LINK = 0U,
T_STATE_LINK1 = 3U,
T_STATE_ENTER_recurser = 1U,
};
int x;
/*temp*/ int recurser_i;
private:
unsigned int (link_stack[10U]);
int (i_stack[10U]);
unsigned int recurser_stack_index;
unsigned int state;
public:
/*temp*/ int temp_i2;
/*temp*/ unsigned int recurser_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i2=(1);
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::recurser_i= ::TopLevel::temp_i2;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
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
 ::TopLevel::temp_i=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::recurser_i= ::TopLevel::temp_i;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
 ::TopLevel::temp_i1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_THEN_ELSE;
 ::TopLevel::recurser_i= ::TopLevel::temp_i1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK_THEN_ELSE )
{
 ::TopLevel::temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
 ::TopLevel::state= ::TopLevel::temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}