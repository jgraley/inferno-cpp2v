#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ unsigned int temp_link;
enum TStates
{
T_STATE_BREAK = 5U,
T_STATE_CASE = 3U,
T_STATE_CASE1 = 4U,
T_STATE_CASE2 = 8U,
T_STATE_ELSE = 13U,
T_STATE_ENTER_f = 15U,
T_STATE_LINK = 12U,
T_STATE_PROCEED = 0U,
T_STATE_PROCEED1 = 1U,
T_STATE_PROCEED2 = 6U,
T_STATE_PROCEED3 = 9U,
T_STATE_PROCEED4 = 10U,
T_STATE_PROCEED_CASE = 2U,
T_STATE_PROCEED_CASE1 = 7U,
T_STATE_THEN = 11U,
T_STATE_THEN_ELSE_BREAK = 14U,
};
void T();
private:
int switch_value;
int switch_value1;
public:
int x;
private:
unsigned int link;
unsigned int state;
public:
/*temp*/ int f_return;
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
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
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
 ::TopLevel::switch_value=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=(((0)== ::TopLevel::switch_value) ?  ::TopLevel::T_STATE_CASE1 :  ::TopLevel::T_STATE_PROCEED);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED== ::TopLevel::state )
{
 ::TopLevel::state=(((4)== ::TopLevel::switch_value) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED1);
}
if(  ::TopLevel::T_STATE_PROCEED1== ::TopLevel::state )
{
 ::TopLevel::state=(((1)== ::TopLevel::switch_value) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if(  ::TopLevel::T_STATE_PROCEED_CASE== ::TopLevel::state )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::T_STATE_CASE== ::TopLevel::state )
{
 ::TopLevel::x=(44);
 ::TopLevel::state= ::TopLevel::T_STATE_CASE1;
}
if(  ::TopLevel::T_STATE_CASE1== ::TopLevel::state )
{
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::T_STATE_BREAK== ::TopLevel::state )
{
 ::TopLevel::switch_value1=(2);
 ::TopLevel::state=(((2)== ::TopLevel::switch_value1) ?  ::TopLevel::T_STATE_CASE2 :  ::TopLevel::T_STATE_PROCEED2);
}
if(  ::TopLevel::T_STATE_PROCEED2== ::TopLevel::state )
{
 ::TopLevel::state=(((1)== ::TopLevel::switch_value1) ?  ::TopLevel::T_STATE_PROCEED_CASE1 :  ::TopLevel::T_STATE_PROCEED_CASE1);
}
if(  ::TopLevel::T_STATE_PROCEED_CASE1== ::TopLevel::state )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if(  ::TopLevel::T_STATE_CASE2== ::TopLevel::state )
{
 ::TopLevel::state=((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED3);
}
if(  ::TopLevel::T_STATE_PROCEED3== ::TopLevel::state )
{
 ::TopLevel::state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED4);
}
if(  ::TopLevel::T_STATE_PROCEED4== ::TopLevel::state )
{
 ::TopLevel::muxtemp=(88);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::T_STATE_THEN== ::TopLevel::state )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::muxtemp= ::TopLevel::result;
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::T_STATE_ELSE== ::TopLevel::state )
{
 ::TopLevel::result1= ::TopLevel::muxtemp;
 ::TopLevel::x= ::TopLevel::result1;
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if(  ::TopLevel::T_STATE_THEN_ELSE_BREAK== ::TopLevel::state )
{
cease(  ::TopLevel::x );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state )
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
