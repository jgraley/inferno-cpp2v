#include "isystemc.h"

class TopLevel;
int i;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_METHOD(T);
}
/*temp*/ int temp_n;
/*temp*/ unsigned int helper_link;
private:
unsigned int link;
public:
void T();
private:
int (n_stack[10U]);
unsigned int helper_stack_index;
unsigned int (link_stack[10U]);
public:
enum TStates
{
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_YIELD = 6U,
T_STATE_PROCEED_NEXT = 5U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_PROCEED_THEN_ELSE_1 = 7U,
T_STATE_YIELD_1 = 2U,
T_STATE_PROCEED_NEXT_1 = 0U,
T_STATE_ENTER_helper = 4U,
T_STATE_LINK = 1U,
T_STATE_LINK_1 = 8U,
};
private:
unsigned int state;
public:
/*temp*/ unsigned int temp_link;
/*temp*/ unsigned int otherhelper_link;
/*temp*/ unsigned int temp_link_1;
/*temp*/ int helper_n;
};
TopLevel top_level("top_level");
int j;
int gvar;

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_1) )
{
 ::gvar=( ::gvar+ ::i);
 ::TopLevel::temp_n=(3);
 ::TopLevel::helper_n= ::TopLevel::temp_n;
 ::TopLevel::helper_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK) )
{
 ::gvar=( ::gvar*(2));
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD_1;
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_YIELD_1) )
{
 ::i=( ::i+(1));
 ::TopLevel::state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE) )
{
cease(  ::gvar );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ENTER_helper) )
{
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
 ::TopLevel::state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT) )
{
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_YIELD) )
{
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
 ::TopLevel::state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1) )
{
 ::TopLevel::otherhelper_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_otherhelper;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK_1) )
{
 ::TopLevel::temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ENTER_otherhelper) )
{
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
 ::TopLevel::temp_link_1= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link_1;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
