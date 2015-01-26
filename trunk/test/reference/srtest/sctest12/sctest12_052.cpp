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
enum TStates
{
T_STATE_PROCEED = 9U,
T_STATE_PROCEED1 = 10U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_PROCEED2 = 0U,
T_STATE_CASE = 3U,
T_STATE_ELSE = 13U,
T_STATE_PROCEED3 = 6U,
T_STATE_ENTER_f = 15U,
T_STATE_PROCEED_CASE = 7U,
T_STATE_CASE1 = 4U,
T_STATE_CASE2 = 8U,
T_STATE_PROCEED_CASE1 = 2U,
T_STATE_THEN = 11U,
T_STATE_LINK = 12U,
T_STATE_BREAK = 5U,
T_STATE_PROCEED4 = 1U,
};
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link1;
private:
unsigned int link;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto int switch_value;
/*temp*/ int result;
auto unsigned int state;
/*temp*/ int muxtemp;
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&PROCEED, &&PROCEED1, &&PROCEED_CASE, &&CASE, &&CASE1, &&BREAK, &&PROCEED2, &&PROCEED_CASE1, &&CASE2, &&PROCEED3, &&PROCEED4, &&THEN, &&LINK, &&ELSE, &&THEN_ELSE_BREAK, &&ENTER_f };
/*temp*/ int result1;
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
state=((switch_value==(0)) ?  ::TopLevel::T_STATE_CASE1 :  ::TopLevel::T_STATE_PROCEED2);
goto *(lmap[state]);
PROCEED:;
state=((switch_value==(4)) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED4);
goto *(lmap[state]);
PROCEED1:;
state=((switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE1 :  ::TopLevel::T_STATE_PROCEED_CASE1);
goto *(lmap[state]);
PROCEED_CASE:;
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_BREAK;
goto *(lmap[state]);
CASE:;
 ::TopLevel::x=(44);
state= ::TopLevel::T_STATE_CASE1;
goto *(lmap[state]);
CASE1:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
state= ::TopLevel::T_STATE_BREAK;
goto *(lmap[state]);
BREAK:;
switch_value1=(2);
state=((switch_value1==(2)) ?  ::TopLevel::T_STATE_CASE2 :  ::TopLevel::T_STATE_PROCEED3);
goto *(lmap[state]);
PROCEED2:;
state=((switch_value1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
goto *(lmap[state]);
PROCEED_CASE1:;
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
goto *(lmap[state]);
CASE2:;
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
PROCEED3:;
state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED1);
goto *(lmap[state]);
PROCEED4:;
muxtemp=(88);
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
THEN:;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
LINK:;
result1= ::TopLevel::f_return;
muxtemp=result1;
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
ELSE:;
result=muxtemp;
 ::TopLevel::x=result;
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
goto *(lmap[state]);
THEN_ELSE_BREAK:;
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
goto *(lmap[state]);
}
