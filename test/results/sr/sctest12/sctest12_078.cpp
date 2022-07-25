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
T_STATE_BREAK = 2,
T_STATE_CASE = 0,
T_STATE_CASE_1 = 4,
T_STATE_CASE_2 = 9,
T_STATE_ELSE = 7,
T_STATE_ENTER_f = 15,
T_STATE_LINK = 13,
T_STATE_PROCEED = 1,
T_STATE_PROCEED_1 = 3,
T_STATE_PROCEED_2 = 5,
T_STATE_PROCEED_3 = 10,
T_STATE_PROCEED_4 = 11,
T_STATE_PROCEED_CASE = 6,
T_STATE_PROCEED_CASE_1 = 8,
T_STATE_THEN = 12,
T_STATE_THEN_ELSE_BREAK = 14,
};
void T();
int x;
private:
unsigned int link;
public:
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&CASE, &&PROCEED, &&BREAK, &&PROCEED_1, &&CASE_1, &&PROCEED_2, &&PROCEED_CASE, &&ELSE, &&PROCEED_CASE_1, &&CASE_2, &&PROCEED_3, &&PROCEED_4, &&THEN, &&LINK, &&THEN_ELSE_BREAK, &&ENTER_f };
auto int switch_value;
auto int switch_value_1;
auto unsigned int state;
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ int result_1;
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
state=(((1)==switch_value) ?  ::TopLevel::T_STATE_PROCEED_CASE_1 :  ::TopLevel::T_STATE_PROCEED_2);
PROCEED_2:;
if(  ::TopLevel::T_STATE_PROCEED_2==state )
{
state=(((0)==switch_value) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED);
}
PROCEED:;
if(  ::TopLevel::T_STATE_PROCEED==state )
{
state=(((4)==switch_value) ?  ::TopLevel::T_STATE_CASE_1 :  ::TopLevel::T_STATE_PROCEED_CASE_1);
}
PROCEED_CASE_1:;
if(  ::TopLevel::T_STATE_PROCEED_CASE_1==state )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_BREAK;
}
CASE_1:;
if(  ::TopLevel::T_STATE_CASE_1==state )
{
 ::TopLevel::x=(44);
state= ::TopLevel::T_STATE_CASE;
}
CASE:;
if(  ::TopLevel::T_STATE_CASE==state )
{
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
state= ::TopLevel::T_STATE_BREAK;
}
BREAK:;
if(  ::TopLevel::T_STATE_BREAK==state )
{
switch_value_1=(2);
state=(((1)==switch_value_1) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_1);
}
PROCEED_1:;
if(  ::TopLevel::T_STATE_PROCEED_1==state )
{
state=(((2)==switch_value_1) ?  ::TopLevel::T_STATE_CASE_2 :  ::TopLevel::T_STATE_PROCEED_CASE);
}
PROCEED_CASE:;
if(  ::TopLevel::T_STATE_PROCEED_CASE==state )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
CASE_2:;
if(  ::TopLevel::T_STATE_CASE_2==state )
{
state=((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_3);
}
PROCEED_3:;
if(  ::TopLevel::T_STATE_PROCEED_3==state )
{
state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_4);
}
PROCEED_4:;
if(  ::TopLevel::T_STATE_PROCEED_4==state )
{
muxtemp=(88);
state= ::TopLevel::T_STATE_ELSE;
}
THEN:;
if(  ::TopLevel::T_STATE_THEN==state )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK:;
if(  ::TopLevel::T_STATE_LINK==state )
{
result= ::TopLevel::f_return;
muxtemp=result;
state= ::TopLevel::T_STATE_ELSE;
}
ELSE:;
if(  ::TopLevel::T_STATE_ELSE==state )
{
result_1=muxtemp;
 ::TopLevel::x=result_1;
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
THEN_ELSE_BREAK:;
if(  ::TopLevel::T_STATE_THEN_ELSE_BREAK==state )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
ENTER_f:;
if(  ::TopLevel::T_STATE_ENTER_f==state )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
