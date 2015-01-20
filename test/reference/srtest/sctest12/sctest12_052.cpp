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
public:
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link_1;
void T();
enum TStates
{
T_STATE_ENTER_f = 15U,
T_STATE_LINK = 12U,
T_STATE_THEN = 11U,
T_STATE_PROCEED = 6U,
T_STATE_PROCEED_CASE = 2U,
T_STATE_PROCEED_1 = 9U,
T_STATE_PROCEED_2 = 0U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_CASE = 3U,
T_STATE_CASE_1 = 4U,
T_STATE_PROCEED_3 = 10U,
T_STATE_PROCEED_4 = 1U,
T_STATE_PROCEED_CASE_1 = 7U,
T_STATE_ELSE = 13U,
T_STATE_BREAK = 5U,
T_STATE_CASE_2 = 8U,
};
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ int result;
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&PROCEED, &&PROCEED_1, &&PROCEED_CASE, &&CASE, &&CASE_1, &&BREAK, &&PROCEED_2, &&PROCEED_CASE_1, &&CASE_2, &&PROCEED_3, &&PROCEED_4, &&THEN, &&LINK, &&ELSE, &&THEN_ELSE_BREAK, &&ENTER_f };
auto int switch_value;
auto int switch_value_1;
/*temp*/ int muxtemp;
/*temp*/ int result_1;
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
state=((switch_value==(0)) ?  ::TopLevel::T_STATE_CASE_1 :  ::TopLevel::T_STATE_PROCEED_2);
goto *(lmap[state]);
PROCEED:;
state=((switch_value==(4)) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED_4);
goto *(lmap[state]);
PROCEED_1:;
state=((switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
goto *(lmap[state]);
PROCEED_CASE:;
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_BREAK;
goto *(lmap[state]);
CASE:;
 ::TopLevel::x=(44);
state= ::TopLevel::T_STATE_CASE_1;
goto *(lmap[state]);
CASE_1:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
state= ::TopLevel::T_STATE_BREAK;
goto *(lmap[state]);
BREAK:;
switch_value_1=(2);
state=((switch_value_1==(2)) ?  ::TopLevel::T_STATE_CASE_2 :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
PROCEED_2:;
state=((switch_value_1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE_1 :  ::TopLevel::T_STATE_PROCEED_CASE_1);
goto *(lmap[state]);
PROCEED_CASE_1:;
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
goto *(lmap[state]);
CASE_2:;
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_1);
goto *(lmap[state]);
PROCEED_3:;
state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_3);
goto *(lmap[state]);
PROCEED_4:;
muxtemp=(88);
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
THEN:;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
LINK:;
result= ::TopLevel::f_return;
muxtemp=result;
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
ELSE:;
result_1=muxtemp;
 ::TopLevel::x=result_1;
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
goto *(lmap[state]);
THEN_ELSE_BREAK:;
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
goto *(lmap[state]);
}
