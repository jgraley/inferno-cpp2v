#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_PROCEED = 6U,
T_STATE_THEN = 11U,
T_STATE_PROCEED1 = 9U,
T_STATE_LINK = 12U,
T_STATE_PROCEED2 = 0U,
T_STATE_CASE = 4U,
T_STATE_ELSE = 13U,
T_STATE_PROCEED3 = 1U,
T_STATE_PROCEED4 = 10U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_PROCEED_CASE = 2U,
T_STATE_CASE1 = 8U,
T_STATE_CASE2 = 3U,
T_STATE_BREAK = 5U,
T_STATE_ENTER_f = 15U,
T_STATE_PROCEED_CASE1 = 7U,
};
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
int x;
/*temp*/ unsigned int f_link1;
private:
unsigned int link;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED, &&PROCEED1, &&PROCEED_CASE, &&CASE, &&CASE1, &&BREAK, &&PROCEED2, &&PROCEED_CASE1, &&CASE2, &&PROCEED3, &&PROCEED4, &&THEN, &&LINK, &&ELSE, &&THEN_ELSE_BREAK, &&ENTER_f };
/*temp*/ int result;
auto unsigned int state;
/*temp*/ int result1;
/*temp*/ int muxtemp;
/*temp*/ unsigned int temp_link;
auto int switch_value;
auto int switch_value1;
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
PROCEED:;
if(  ::TopLevel::T_STATE_PROCEED2==state )
{
state=((switch_value==(4)) ?  ::TopLevel::T_STATE_CASE2 :  ::TopLevel::T_STATE_PROCEED3);
}
PROCEED1:;
if( state== ::TopLevel::T_STATE_PROCEED3 )
{
state=((switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
PROCEED_CASE:;
if(  ::TopLevel::T_STATE_PROCEED_CASE==state )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_BREAK;
}
CASE:;
if( state== ::TopLevel::T_STATE_CASE2 )
{
 ::TopLevel::x=(44);
state= ::TopLevel::T_STATE_CASE;
}
CASE1:;
if( state== ::TopLevel::T_STATE_CASE )
{
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
state= ::TopLevel::T_STATE_BREAK;
}
BREAK:;
if( state== ::TopLevel::T_STATE_BREAK )
{
switch_value1=(2);
state=((switch_value1==(2)) ?  ::TopLevel::T_STATE_CASE1 :  ::TopLevel::T_STATE_PROCEED);
}
PROCEED2:;
if( state== ::TopLevel::T_STATE_PROCEED )
{
state=((switch_value1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE1 :  ::TopLevel::T_STATE_PROCEED_CASE1);
}
PROCEED_CASE1:;
if( state== ::TopLevel::T_STATE_PROCEED_CASE1 )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
CASE2:;
if( state== ::TopLevel::T_STATE_CASE1 )
{
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED1);
}
PROCEED3:;
if( state== ::TopLevel::T_STATE_PROCEED1 )
{
state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED4);
}
PROCEED4:;
if( state== ::TopLevel::T_STATE_PROCEED4 )
{
muxtemp=(88);
state= ::TopLevel::T_STATE_ELSE;
}
THEN:;
if( state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK:;
if( state== ::TopLevel::T_STATE_LINK )
{
result= ::TopLevel::f_return;
muxtemp=result;
state= ::TopLevel::T_STATE_ELSE;
}
ELSE:;
if( state== ::TopLevel::T_STATE_ELSE )
{
result1=muxtemp;
 ::TopLevel::x=result1;
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
THEN_ELSE_BREAK:;
if( state== ::TopLevel::T_STATE_THEN_ELSE_BREAK )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
ENTER_f:;
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}