#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
/*temp*/ unsigned int temp_link;
enum TStates
{
T_STATE_BREAK = 2,
T_STATE_CASE = 0,
T_STATE_CASE_1 = 4,
T_STATE_CASE_2 = 9,
T_STATE_ELSE = 7,
T_STATE_ENTER_f = 15,
T_STATE_LINK = 13,
T_STATE_PROCEED = 1,
T_STATE_PROCEED_1 = 3,
T_STATE_PROCEED_2 = 5,
T_STATE_PROCEED_3 = 10,
T_STATE_PROCEED_4 = 11,
T_STATE_PROCEED_CASE = 6,
T_STATE_PROCEED_CASE_1 = 8,
T_STATE_THEN = 12,
T_STATE_THEN_ELSE_BREAK = 14,
};
void T();
private:
int switch_value;
int switch_value_1;
public:
int x;
private:
unsigned int link;
unsigned int state;
public:
/*temp*/ int f_return;
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0) )
{
 ::TopLevel::x=(0);
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 0:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
 ::TopLevel::switch_value=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=(((1)== ::TopLevel::switch_value) ?  ::TopLevel::T_STATE_PROCEED_CASE_1 :  ::TopLevel::T_STATE_PROCEED_2);
enabled=(false);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_2== ::TopLevel::state) )
{
 ::TopLevel::state=(((0)== ::TopLevel::switch_value) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED== ::TopLevel::state) )
{
 ::TopLevel::state=(((4)== ::TopLevel::switch_value) ?  ::TopLevel::T_STATE_CASE_1 :  ::TopLevel::T_STATE_PROCEED_CASE_1);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_CASE_1== ::TopLevel::state) )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if( enabled&&( ::TopLevel::T_STATE_CASE_1== ::TopLevel::state) )
{
 ::TopLevel::x=(44);
 ::TopLevel::state= ::TopLevel::T_STATE_CASE;
}
if( enabled&&( ::TopLevel::T_STATE_CASE== ::TopLevel::state) )
{
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if( enabled&&( ::TopLevel::T_STATE_BREAK== ::TopLevel::state) )
{
 ::TopLevel::switch_value_1=(2);
 ::TopLevel::state=(((1)== ::TopLevel::switch_value_1) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_1);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_1== ::TopLevel::state) )
{
 ::TopLevel::state=(((2)== ::TopLevel::switch_value_1) ?  ::TopLevel::T_STATE_CASE_2 :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_CASE== ::TopLevel::state) )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if( enabled&&( ::TopLevel::T_STATE_CASE_2== ::TopLevel::state) )
{
 ::TopLevel::state=((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_3);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_3== ::TopLevel::state) )
{
 ::TopLevel::state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_4);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_4== ::TopLevel::state) )
{
 ::TopLevel::muxtemp=(88);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if( enabled&&( ::TopLevel::T_STATE_THEN== ::TopLevel::state) )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::T_STATE_LINK== ::TopLevel::state) )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::muxtemp= ::TopLevel::result;
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if( enabled&&( ::TopLevel::T_STATE_ELSE== ::TopLevel::state) )
{
 ::TopLevel::result_1= ::TopLevel::muxtemp;
 ::TopLevel::x= ::TopLevel::result_1;
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if( enabled&&( ::TopLevel::T_STATE_THEN_ELSE_BREAK== ::TopLevel::state) )
{
cease(  ::TopLevel::x );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state) )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
