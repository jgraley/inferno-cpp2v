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
void *link;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
static const void *(lmap[]) = { &&CASE, &&PROCEED, &&BREAK, &&PROCEED_1, &&CASE_1, &&PROCEED_2, &&PROCEED_CASE, &&ELSE, &&PROCEED_CASE_1, &&CASE_2, &&PROCEED_3, &&PROCEED_4, &&THEN, &&LINK, &&THEN_ELSE_BREAK, &&ENTER_f };
auto int switch_value;
auto int switch_value_1;
auto void *state;
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
{
state=(((1)==switch_value) ? (lmap[ ::TopLevel::T_STATE_PROCEED_CASE_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_2]));
goto *(state);
}
PROCEED_2:;
{
state=(((0)==switch_value) ? (lmap[ ::TopLevel::T_STATE_CASE]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED:;
{
state=(((4)==switch_value) ? (lmap[ ::TopLevel::T_STATE_CASE_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_CASE_1]));
goto *(state);
}
PROCEED_CASE_1:;
 ::TopLevel::x=(99);
{
state=(lmap[ ::TopLevel::T_STATE_BREAK]);
goto *(state);
}
CASE_1:;
 ::TopLevel::x=(44);
{
state=(lmap[ ::TopLevel::T_STATE_CASE]);
goto *(state);
}
CASE:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
{
state=(lmap[ ::TopLevel::T_STATE_BREAK]);
goto *(state);
}
BREAK:;
switch_value_1=(2);
{
state=(((1)==switch_value_1) ? (lmap[ ::TopLevel::T_STATE_PROCEED_CASE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_1]));
goto *(state);
}
PROCEED_1:;
{
state=(((2)==switch_value_1) ? (lmap[ ::TopLevel::T_STATE_CASE_2]) : (lmap[ ::TopLevel::T_STATE_PROCEED_CASE]));
goto *(state);
}
PROCEED_CASE:;
 ::TopLevel::x=(99);
{
state=(lmap[ ::TopLevel::T_STATE_THEN_ELSE_BREAK]);
goto *(state);
}
CASE_2:;
{
state=((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ? (lmap[ ::TopLevel::T_STATE_THEN_ELSE_BREAK]) : (lmap[ ::TopLevel::T_STATE_PROCEED_3]));
goto *(state);
}
PROCEED_3:;
{
state=((!(false)) ? (lmap[ ::TopLevel::T_STATE_THEN]) : (lmap[ ::TopLevel::T_STATE_PROCEED_4]));
goto *(state);
}
PROCEED_4:;
muxtemp=(88);
{
state=(lmap[ ::TopLevel::T_STATE_ELSE]);
goto *(state);
}
THEN:;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK:;
result= ::TopLevel::f_return;
muxtemp=result;
{
state=(lmap[ ::TopLevel::T_STATE_ELSE]);
goto *(state);
}
ELSE:;
result_1=muxtemp;
 ::TopLevel::x=result_1;
{
state=(lmap[ ::TopLevel::T_STATE_THEN_ELSE_BREAK]);
goto *(state);
}
THEN_ELSE_BREAK:;
cease(  ::TopLevel::x );
return ;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
