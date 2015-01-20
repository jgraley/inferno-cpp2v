#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
private:
int switch_value;
unsigned int link;
public:
/*temp*/ int result;
/*temp*/ unsigned int f_link;
/*temp*/ int result_1;
private:
int switch_value_1;
public:
/*temp*/ int muxtemp;
/*temp*/ unsigned int temp_link;
enum TStates
{
T_STATE_PROCEED_CASE = 7U,
T_STATE_PROCEED = 6U,
T_STATE_PROCEED_1 = 0U,
T_STATE_CASE = 8U,
T_STATE_PROCEED_2 = 10U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_BREAK = 5U,
T_STATE_ELSE = 13U,
T_STATE_PROCEED_3 = 1U,
T_STATE_ENTER_f = 15U,
T_STATE_THEN = 11U,
T_STATE_CASE_1 = 4U,
T_STATE_PROCEED_CASE_1 = 2U,
T_STATE_PROCEED_4 = 9U,
T_STATE_CASE_2 = 3U,
T_STATE_LINK = 12U,
};
private:
unsigned int state;
public:
int x;
/*temp*/ int f_return;
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 0:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
 ::TopLevel::switch_value_1=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=(( ::TopLevel::switch_value_1==(0)) ?  ::TopLevel::T_STATE_CASE_1 :  ::TopLevel::T_STATE_PROCEED_1);
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_1) )
{
 ::TopLevel::state=(( ::TopLevel::switch_value_1==(4)) ?  ::TopLevel::T_STATE_CASE_2 :  ::TopLevel::T_STATE_PROCEED_3);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_3) )
{
 ::TopLevel::state=(( ::TopLevel::switch_value_1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE_1 :  ::TopLevel::T_STATE_PROCEED_CASE_1);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE_1) )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_CASE_2) )
{
 ::TopLevel::x=(44);
 ::TopLevel::state= ::TopLevel::T_STATE_CASE_1;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_CASE_1) )
{
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_BREAK) )
{
 ::TopLevel::switch_value=(2);
 ::TopLevel::state=(( ::TopLevel::switch_value==(2)) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED) )
{
 ::TopLevel::state=(( ::TopLevel::switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE) )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_CASE) )
{
 ::TopLevel::state=((!(( ::TopLevel::x==(0))||((2)== ::TopLevel::x))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_4);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_4) )
{
 ::TopLevel::state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_2);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_2) )
{
 ::TopLevel::muxtemp=(88);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_THEN) )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK) )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::muxtemp= ::TopLevel::result;
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ELSE) )
{
 ::TopLevel::result_1= ::TopLevel::muxtemp;
 ::TopLevel::x= ::TopLevel::result_1;
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_THEN_ELSE_BREAK) )
{
cease(  ::TopLevel::x );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f) )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
