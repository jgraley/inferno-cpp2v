#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ unsigned int f_link;
private:
unsigned int link;
public:
/*temp*/ int f_return;
void T();
enum TStates
{
T_STATE_LINK = 12U,
T_STATE_PROCEED = 9U,
T_STATE_CASE = 4U,
T_STATE_PROCEED1 = 1U,
T_STATE_PROCEED_CASE = 2U,
T_STATE_PROCEED2 = 0U,
T_STATE_PROCEED3 = 10U,
T_STATE_CASE1 = 3U,
T_STATE_BREAK = 5U,
T_STATE_ENTER_f = 15U,
T_STATE_PROCEED4 = 6U,
T_STATE_CASE2 = 8U,
T_STATE_THEN = 11U,
T_STATE_ELSE = 13U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_PROCEED_CASE1 = 7U,
};
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
auto unsigned int state;
auto int switch_value;
/*temp*/ int result1;
auto int switch_value1;
/*temp*/ int muxtemp;
/*temp*/ unsigned int temp_link;
do
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
switch_value=(0);
wait(SC_ZERO_TIME);
state=((switch_value==(0)) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED2);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED2==state )
{
state=((switch_value==(4)) ?  ::TopLevel::T_STATE_CASE1 :  ::TopLevel::T_STATE_PROCEED1);
}
if( state== ::TopLevel::T_STATE_PROCEED1 )
{
state=((switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if(  ::TopLevel::T_STATE_PROCEED_CASE==state )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_BREAK;
}
if( state== ::TopLevel::T_STATE_CASE1 )
{
 ::TopLevel::x=(44);
state= ::TopLevel::T_STATE_CASE;
}
if( state== ::TopLevel::T_STATE_CASE )
{
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
state= ::TopLevel::T_STATE_BREAK;
}
if( state== ::TopLevel::T_STATE_BREAK )
{
switch_value1=(2);
state=((switch_value1==(2)) ?  ::TopLevel::T_STATE_CASE2 :  ::TopLevel::T_STATE_PROCEED4);
}
if( state== ::TopLevel::T_STATE_PROCEED4 )
{
state=((switch_value1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE1 :  ::TopLevel::T_STATE_PROCEED_CASE1);
}
if( state== ::TopLevel::T_STATE_PROCEED_CASE1 )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if( state== ::TopLevel::T_STATE_CASE2 )
{
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED);
}
if( state== ::TopLevel::T_STATE_PROCEED )
{
state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED3);
}
if( state== ::TopLevel::T_STATE_PROCEED3 )
{
muxtemp=(88);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result= ::TopLevel::f_return;
muxtemp=result;
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result1=muxtemp;
 ::TopLevel::x=result1;
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE_BREAK )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}