#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *f_link;
enum TStates
{
T_STATE_ENTER_f = 15U,
T_STATE_PROCEED_CASE = 7U,
T_STATE_LINK = 12U,
T_STATE_BREAK = 5U,
T_STATE_PROCEED_CASE_1 = 2U,
T_STATE_CASE = 4U,
T_STATE_PROCEED = 6U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_PROCEED_1 = 1U,
T_STATE_CASE_1 = 3U,
T_STATE_ELSE = 13U,
T_STATE_THEN = 11U,
T_STATE_PROCEED_2 = 9U,
T_STATE_CASE_2 = 8U,
T_STATE_PROCEED_3 = 10U,
T_STATE_PROCEED_4 = 0U,
};
/*temp*/ void *f_link_1;
private:
void *link;
public:
/*temp*/ int f_return;
void T();
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const void *(lmap[]) = { &&PROCEED, &&PROCEED_1, &&PROCEED_CASE, &&CASE, &&CASE_1, &&BREAK, &&PROCEED_2, &&PROCEED_CASE_1, &&CASE_2, &&PROCEED_3, &&PROCEED_4, &&THEN, &&LINK, &&ELSE, &&THEN_ELSE_BREAK, &&ENTER_f };
/*temp*/ int result;
auto int switch_value;
/*temp*/ void *temp_link;
/*temp*/ int result_1;
/*temp*/ int muxtemp;
auto void *state;
auto int switch_value_1;
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
switch_value_1=(0);
wait(SC_ZERO_TIME);
{
state=((switch_value_1==(0)) ? (lmap[ ::TopLevel::T_STATE_CASE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_4]));
goto *(state);
}
PROCEED:;
{
state=((switch_value_1==(4)) ? (lmap[ ::TopLevel::T_STATE_CASE_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_1]));
goto *(state);
}
PROCEED_1:;
{
state=((switch_value_1==(1)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_CASE_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_CASE_1]));
goto *(state);
}
PROCEED_CASE:;
 ::TopLevel::x=(99);
{
state=(lmap[ ::TopLevel::T_STATE_BREAK]);
goto *(state);
}
CASE:;
 ::TopLevel::x=(44);
{
state=(lmap[ ::TopLevel::T_STATE_CASE]);
goto *(state);
}
CASE_1:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
{
state=(lmap[ ::TopLevel::T_STATE_BREAK]);
goto *(state);
}
BREAK:;
switch_value=(2);
{
state=((switch_value==(2)) ? (lmap[ ::TopLevel::T_STATE_CASE_2]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED_2:;
{
state=((switch_value==(1)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_CASE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_CASE]));
goto *(state);
}
PROCEED_CASE_1:;
 ::TopLevel::x=(99);
{
state=(lmap[ ::TopLevel::T_STATE_THEN_ELSE_BREAK]);
goto *(state);
}
CASE_2:;
{
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (lmap[ ::TopLevel::T_STATE_THEN_ELSE_BREAK]) : (lmap[ ::TopLevel::T_STATE_PROCEED_2]));
goto *(state);
}
PROCEED_3:;
{
state=((!(false)) ? (lmap[ ::TopLevel::T_STATE_THEN]) : (lmap[ ::TopLevel::T_STATE_PROCEED_3]));
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
result_1= ::TopLevel::f_return;
muxtemp=result_1;
{
state=(lmap[ ::TopLevel::T_STATE_ELSE]);
goto *(state);
}
ELSE:;
result=muxtemp;
 ::TopLevel::x=result;
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
