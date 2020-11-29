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
T_STATE_BREAK = 5U,
T_STATE_CASE2 = 3U,
T_STATE_CASE1 = 4U,
T_STATE_CASE = 8U,
T_STATE_ELSE = 13U,
T_STATE_ENTER_f = 15U,
T_STATE_LINK = 12U,
T_STATE_PROCEED3 = 0U,
T_STATE_PROCEED2 = 1U,
T_STATE_PROCEED = 6U,
T_STATE_PROCEED1 = 9U,
T_STATE_PROCEED4 = 10U,
T_STATE_PROCEED_CASE1 = 2U,
T_STATE_PROCEED_CASE = 7U,
T_STATE_THEN = 11U,
T_STATE_THEN_ELSE_BREAK = 14U,
};
void T();
private:
unsigned int link;
unsigned int state;
int switch_value;
int switch_value1;
public:
int x;
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
/*temp*/ int muxtemp;
/*temp*/ int result1;
/*temp*/ int result;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
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
 ::TopLevel::switch_value=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=(( ::TopLevel::switch_value==(0)) ?  ::TopLevel::T_STATE_CASE1 :  ::TopLevel::T_STATE_PROCEED3);
return ;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED3 )
{
 ::TopLevel::state=(( ::TopLevel::switch_value==(4)) ?  ::TopLevel::T_STATE_CASE2 :  ::TopLevel::T_STATE_PROCEED2);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED2 )
{
 ::TopLevel::state=(( ::TopLevel::switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE1 :  ::TopLevel::T_STATE_PROCEED_CASE1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE1 )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE2 )
{
 ::TopLevel::x=(44);
 ::TopLevel::state= ::TopLevel::T_STATE_CASE1;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE1 )
{
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_BREAK )
{
 ::TopLevel::switch_value1=(2);
 ::TopLevel::state=(( ::TopLevel::switch_value1==(2)) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::state=(( ::TopLevel::switch_value1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE )
{
 ::TopLevel::state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED1 )
{
 ::TopLevel::state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED4);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED4 )
{
 ::TopLevel::muxtemp=(88);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::muxtemp= ::TopLevel::result;
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ELSE )
{
 ::TopLevel::result1= ::TopLevel::muxtemp;
 ::TopLevel::x= ::TopLevel::result1;
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN_ELSE_BREAK )
{
cease(  ::TopLevel::x );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
next_trigger(SC_ZERO_TIME);
}
