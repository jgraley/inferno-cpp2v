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
enum TStates
{
T_STATE_CASE = 4U,
T_STATE_PROCEED_CASE = 2U,
T_STATE_PROCEED = 0U,
T_STATE_ELSE = 13U,
T_STATE_ENTER_f = 15U,
T_STATE_BREAK = 5U,
T_STATE_PROCEED_1 = 9U,
T_STATE_THEN = 11U,
T_STATE_PROCEED_CASE_1 = 7U,
T_STATE_CASE_1 = 8U,
T_STATE_PROCEED_2 = 1U,
T_STATE_CASE_2 = 3U,
T_STATE_PROCEED_3 = 10U,
T_STATE_PROCEED_4 = 6U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_LINK = 12U,
};
/*temp*/ int f_return;
void T();
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result_1;
auto int switch_value;
/*temp*/ int muxtemp;
/*temp*/ unsigned int temp_link;
auto unsigned int state;
auto int switch_value_1;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 0:;
if( ((0)== ::TopLevel::x)||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
switch_value=(0);
wait(SC_ZERO_TIME);
state=(((0)==switch_value) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED);
continue;
}
if( state== ::TopLevel::T_STATE_PROCEED )
{
state=((switch_value==(4)) ?  ::TopLevel::T_STATE_CASE_2 :  ::TopLevel::T_STATE_PROCEED_2);
}
if( state== ::TopLevel::T_STATE_PROCEED_2 )
{
state=(((1)==switch_value) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if( state== ::TopLevel::T_STATE_PROCEED_CASE )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_BREAK;
}
if( state== ::TopLevel::T_STATE_CASE_2 )
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
switch_value_1=(2);
state=((switch_value_1==(2)) ?  ::TopLevel::T_STATE_CASE_1 :  ::TopLevel::T_STATE_PROCEED_4);
}
if( state== ::TopLevel::T_STATE_PROCEED_4 )
{
state=((switch_value_1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE_1 :  ::TopLevel::T_STATE_PROCEED_CASE_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_CASE_1 )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if( state== ::TopLevel::T_STATE_CASE_1 )
{
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_1 )
{
state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_3);
}
if( state== ::TopLevel::T_STATE_PROCEED_3 )
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
result_1= ::TopLevel::f_return;
muxtemp=result_1;
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result=muxtemp;
 ::TopLevel::x=result;
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
wait(SC_ZERO_TIME);
}
while( true );
}
