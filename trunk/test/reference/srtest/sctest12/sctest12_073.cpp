#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
private:
int switch_value;
public:
/*temp*/ int result;
/*temp*/ int result1;
private:
unsigned int state;
public:
/*temp*/ int f_return;
void T();
enum TStates
{
T_STATE_ELSE = 13U,
T_STATE_PROCEED = 0U,
T_STATE_LINK = 12U,
T_STATE_ENTER_f = 15U,
T_STATE_PROCEED1 = 1U,
T_STATE_CASE = 4U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_PROCEED_CASE = 7U,
T_STATE_PROCEED2 = 9U,
T_STATE_THEN = 11U,
T_STATE_PROCEED3 = 10U,
T_STATE_CASE1 = 8U,
T_STATE_PROCEED4 = 6U,
T_STATE_CASE2 = 3U,
T_STATE_PROCEED_CASE1 = 2U,
T_STATE_BREAK = 5U,
};
private:
unsigned int link;
public:
/*temp*/ unsigned int temp_link;
private:
int switch_value1;
public:
/*temp*/ unsigned int f_link;
/*temp*/ int muxtemp;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
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
 ::TopLevel::switch_value1=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=(( ::TopLevel::switch_value1==(0)) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED);
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::state=(( ::TopLevel::switch_value1==(4)) ?  ::TopLevel::T_STATE_CASE2 :  ::TopLevel::T_STATE_PROCEED1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED1 )
{
 ::TopLevel::state=(( ::TopLevel::switch_value1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE1 :  ::TopLevel::T_STATE_PROCEED_CASE1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE1 )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE2 )
{
 ::TopLevel::x=(44);
 ::TopLevel::state= ::TopLevel::T_STATE_CASE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE )
{
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_BREAK )
{
 ::TopLevel::switch_value=(2);
 ::TopLevel::state=(( ::TopLevel::switch_value==(2)) ?  ::TopLevel::T_STATE_CASE1 :  ::TopLevel::T_STATE_PROCEED4);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED4 )
{
 ::TopLevel::state=(( ::TopLevel::switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE1 )
{
 ::TopLevel::state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED2);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED2 )
{
 ::TopLevel::state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED3);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED3 )
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
 ::TopLevel::result1= ::TopLevel::f_return;
 ::TopLevel::muxtemp= ::TopLevel::result1;
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ELSE )
{
 ::TopLevel::result= ::TopLevel::muxtemp;
 ::TopLevel::x= ::TopLevel::result;
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
wait(SC_ZERO_TIME);
}
while( true );
}
