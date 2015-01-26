#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
private:
int switch_value;
public:
void T();
private:
unsigned int link;
public:
enum TStates
{
T_STATE_LINK = 12U,
T_STATE_ENTER_f = 15U,
T_STATE_PROCEED = 10U,
T_STATE_THEN = 11U,
T_STATE_PROCEED_CASE = 2U,
T_STATE_CASE = 3U,
T_STATE_PROCEED1 = 6U,
T_STATE_BREAK = 5U,
T_STATE_ELSE = 13U,
T_STATE_PROCEED2 = 1U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_PROCEED3 = 0U,
T_STATE_PROCEED_CASE1 = 7U,
T_STATE_CASE1 = 4U,
T_STATE_CASE2 = 8U,
T_STATE_PROCEED4 = 9U,
};
private:
unsigned int state;
public:
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
private:
int switch_value1;
public:
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int muxtemp;
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
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
 ::TopLevel::switch_value=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=(( ::TopLevel::switch_value==(0)) ?  ::TopLevel::T_STATE_CASE1 :  ::TopLevel::T_STATE_PROCEED3);
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED3 )
{
 ::TopLevel::state=(( ::TopLevel::switch_value==(4)) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED2);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED2 )
{
 ::TopLevel::state=(( ::TopLevel::switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE )
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
 ::TopLevel::state=(( ::TopLevel::switch_value1==(2)) ?  ::TopLevel::T_STATE_CASE2 :  ::TopLevel::T_STATE_PROCEED1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED1 )
{
 ::TopLevel::state=(( ::TopLevel::switch_value1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE1 :  ::TopLevel::T_STATE_PROCEED_CASE1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE1 )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE2 )
{
 ::TopLevel::state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED4);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED4 )
{
 ::TopLevel::state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
muxtemp=(88);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
result1= ::TopLevel::f_return;
muxtemp=result1;
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ELSE )
{
result=muxtemp;
 ::TopLevel::x=result;
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
temp_link= ::TopLevel::link;
 ::TopLevel::state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
