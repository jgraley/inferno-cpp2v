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
void T();
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_n;
enum TStates
{
T_STATE_PROCEED_NEXT = 5U,
T_STATE_PROCEED_THEN_ELSE = 7U,
T_STATE_YIELD = 6U,
T_STATE_LINK = 8U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_LINK1 = 1U,
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_YIELD1 = 2U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_ENTER_helper = 4U,
};
/*temp*/ unsigned int otherhelper_link;
private:
unsigned int state;
public:
/*temp*/ unsigned int temp_link1;
private:
unsigned int (link_stack[10U]);
int (n_stack[10U]);
public:
/*temp*/ int helper_n;
private:
unsigned int link;
unsigned int helper_stack_index;
public:
/*temp*/ unsigned int helper_link;
};
TopLevel top_level("top_level");
int j;
int gvar;

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
 ::gvar=(1);
 ::i=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
enabled=(false);
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
 ::gvar=( ::gvar+ ::i);
 ::TopLevel::temp_n=(3);
 ::TopLevel::helper_n= ::TopLevel::temp_n;
 ::TopLevel::helper_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
 ::gvar=( ::gvar*(2));
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD1;
enabled=(false);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD1 )
{
 ::i=( ::i+(1));
 ::TopLevel::state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 )
{
cease(  ::gvar );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_helper;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_helper )
{
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
 ::TopLevel::state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_YIELD;
enabled=(false);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
 ::TopLevel::state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
 ::TopLevel::otherhelper_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_otherhelper;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::temp_link1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
 ::TopLevel::state= ::TopLevel::temp_link1;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_otherhelper )
{
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}