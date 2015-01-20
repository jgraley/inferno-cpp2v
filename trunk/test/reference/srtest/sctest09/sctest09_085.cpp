#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
recurser_stack_index(0U)
{
SC_METHOD(T);
}
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i;
void T();
private:
unsigned int recurser_stack_index;
unsigned int state;
public:
/*temp*/ int temp_i_1;
/*temp*/ int recurser_i;
private:
unsigned int (link_stack[10U]);
public:
/*temp*/ unsigned int recurser_link;
/*temp*/ int temp_i_2;
private:
int (i_stack[10U]);
public:
enum TStates
{
T_STATE_PROCEED = 2U,
T_STATE_LINK = 3U,
T_STATE_LINK_THEN_ELSE = 4U,
T_STATE_LINK_1 = 0U,
T_STATE_ENTER_recurser = 1U,
};
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i_1=(1);
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::recurser_i= ::TopLevel::temp_i_1;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK_1) )
{
cease(  ::TopLevel::x );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ENTER_recurser) )
{
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
 ::TopLevel::x++;
 ::TopLevel::state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED) )
{
 ::TopLevel::temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::recurser_i= ::TopLevel::temp_i;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK) )
{
 ::TopLevel::temp_i_2=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_THEN_ELSE;
 ::TopLevel::recurser_i= ::TopLevel::temp_i_2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK_THEN_ELSE) )
{
 ::TopLevel::temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
