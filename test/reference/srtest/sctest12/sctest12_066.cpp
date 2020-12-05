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
unsigned int link;
public:
int x;
/*temp*/ unsigned int f_link1;
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED, &&ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED };
auto unsigned int state;
auto int switch_value;
auto int switch_value1;
/*temp*/ int muxtemp;
/*temp*/ int result1;
/*temp*/ int result;
ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED:;
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
switch_value=(0);
wait(SC_ZERO_TIME);
state=(((0)==switch_value) ?  ::TopLevel::T_STATE_CASE1 :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
}
if(  ::TopLevel::T_STATE_PROCEED==state )
{
state=(((4)==switch_value) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED1);
}
if(  ::TopLevel::T_STATE_PROCEED1==state )
{
state=(((1)==switch_value) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if(  ::TopLevel::T_STATE_PROCEED_CASE==state )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::T_STATE_CASE==state )
{
 ::TopLevel::x=(44);
state= ::TopLevel::T_STATE_CASE1;
}
if(  ::TopLevel::T_STATE_CASE1==state )
{
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
state= ::TopLevel::T_STATE_BREAK;
}
if(  ::TopLevel::T_STATE_BREAK==state )
{
switch_value1=(2);
state=(((2)==switch_value1) ?  ::TopLevel::T_STATE_CASE2 :  ::TopLevel::T_STATE_PROCEED2);
}
if(  ::TopLevel::T_STATE_PROCEED2==state )
{
state=(((1)==switch_value1) ?  ::TopLevel::T_STATE_PROCEED_CASE1 :  ::TopLevel::T_STATE_PROCEED_CASE1);
}
if(  ::TopLevel::T_STATE_PROCEED_CASE1==state )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if(  ::TopLevel::T_STATE_CASE2==state )
{
state=((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED3);
}
if(  ::TopLevel::T_STATE_PROCEED3==state )
{
state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED4);
}
if(  ::TopLevel::T_STATE_PROCEED4==state )
{
muxtemp=(88);
state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::T_STATE_THEN==state )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK==state )
{
result= ::TopLevel::f_return;
muxtemp=result;
state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::T_STATE_ELSE==state )
{
result1=muxtemp;
 ::TopLevel::x=result1;
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if(  ::TopLevel::T_STATE_THEN_ELSE_BREAK==state )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f==state )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
