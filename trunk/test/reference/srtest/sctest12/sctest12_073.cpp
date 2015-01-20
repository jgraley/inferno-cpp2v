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
unsigned int link;
unsigned int state;
public:
/*temp*/ int f_return;
/*temp*/ int muxtemp;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
private:
int switch_value;
public:
/*temp*/ int result_1;
enum TStates
{
T_STATE_BREAK = 5U,
T_STATE_PROCEED_CASE = 7U,
T_STATE_PROCEED_CASE_1 = 2U,
T_STATE_PROCEED = 1U,
T_STATE_ENTER_f = 15U,
T_STATE_LINK = 12U,
T_STATE_PROCEED_1 = 9U,
T_STATE_PROCEED_2 = 6U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_ELSE = 13U,
T_STATE_CASE = 3U,
T_STATE_PROCEED_3 = 10U,
T_STATE_CASE_1 = 8U,
T_STATE_CASE_2 = 4U,
T_STATE_PROCEED_4 = 0U,
T_STATE_THEN = 11U,
};
private:
int switch_value_1;
public:
int x;
void T();
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
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
 ::TopLevel::switch_value_1=(0);
wait(SC_ZERO_TIME);
 ::TopLevel::state=(((0)== ::TopLevel::switch_value_1) ?  ::TopLevel::T_STATE_CASE_2 :  ::TopLevel::T_STATE_PROCEED_4);
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_4 )
{
 ::TopLevel::state=(((4)== ::TopLevel::switch_value_1) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::state=(((1)== ::TopLevel::switch_value_1) ?  ::TopLevel::T_STATE_PROCEED_CASE_1 :  ::TopLevel::T_STATE_PROCEED_CASE_1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE_1 )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE )
{
 ::TopLevel::x=(44);
 ::TopLevel::state= ::TopLevel::T_STATE_CASE_2;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE_2 )
{
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
 ::TopLevel::state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_BREAK )
{
 ::TopLevel::switch_value=(2);
 ::TopLevel::state=(( ::TopLevel::switch_value==(2)) ?  ::TopLevel::T_STATE_CASE_1 :  ::TopLevel::T_STATE_PROCEED_2);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_2 )
{
 ::TopLevel::state=(( ::TopLevel::switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_CASE )
{
 ::TopLevel::x=(99);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_CASE_1 )
{
 ::TopLevel::state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_1 )
{
 ::TopLevel::state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_3);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_3 )
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
 ::TopLevel::result_1= ::TopLevel::f_return;
 ::TopLevel::muxtemp= ::TopLevel::result_1;
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
