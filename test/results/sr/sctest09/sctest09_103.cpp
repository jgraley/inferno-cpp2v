#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
recurser_stack_index(0)
{
SC_METHOD(T);
}
/*temp*/ unsigned int temp_link;
enum TStates
{
T_STATE_ENTER_recurser = 4,
T_STATE_LINK = 1,
T_STATE_LINK_1 = 2,
T_STATE_LINK_THEN_ELSE = 3,
T_STATE_PROCEED = 0,
};
void T();
private:
int (i_stack[10]);
unsigned int (link_stack[10]);
public:
int x;
private:
unsigned int recurser_stack_index;
unsigned int state;
public:
/*temp*/ int recurser_i;
/*temp*/ int temp_i;
/*temp*/ int temp_i_1;
/*temp*/ int temp_i_2;
/*temp*/ unsigned int recurser_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i=(1);
 ::TopLevel::recurser_i= ::TopLevel::temp_i;
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_1;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
enabled=(false);
}
if( enabled&&( ::TopLevel::T_STATE_LINK_1== ::TopLevel::state) )
{
cease(  ::TopLevel::x );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( enabled&&( ::TopLevel::T_STATE_ENTER_recurser== ::TopLevel::state) )
{
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
 ::TopLevel::state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED== ::TopLevel::state) )
{
 ::TopLevel::temp_i_1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_i= ::TopLevel::temp_i_1;
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( enabled&&( ::TopLevel::T_STATE_LINK== ::TopLevel::state) )
{
 ::TopLevel::temp_i_2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_i= ::TopLevel::temp_i_2;
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_THEN_ELSE;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( enabled&&( ::TopLevel::T_STATE_LINK_THEN_ELSE== ::TopLevel::state) )
{
 ::TopLevel::temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
