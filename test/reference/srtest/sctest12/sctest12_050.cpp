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
int x;
private:
void *link;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
static const void *(lmap[]) = { &&PROCEED, &&PROCEED1, &&PROCEED_CASE, &&CASE, &&CASE1, &&BREAK, &&PROCEED2, &&PROCEED_CASE1, &&CASE2, &&PROCEED3, &&PROCEED4, &&THEN, &&LINK, &&ELSE, &&THEN_ELSE_BREAK, &&ENTER_f };
auto int switch_value;
auto int switch_value1;
auto void *state;
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ int result1;
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
state=(((0)==switch_value) ? (lmap[ ::TopLevel::T_STATE_CASE1]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED:;
{
state=(((4)==switch_value) ? (lmap[ ::TopLevel::T_STATE_CASE]) : (lmap[ ::TopLevel::T_STATE_PROCEED1]));
goto *(state);
}
PROCEED1:;
{
state=(((1)==switch_value) ? (lmap[ ::TopLevel::T_STATE_PROCEED_CASE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_CASE]));
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
state=(lmap[ ::TopLevel::T_STATE_CASE1]);
goto *(state);
}
CASE1:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
{
state=(lmap[ ::TopLevel::T_STATE_BREAK]);
goto *(state);
}
BREAK:;
switch_value1=(2);
{
state=(((2)==switch_value1) ? (lmap[ ::TopLevel::T_STATE_CASE2]) : (lmap[ ::TopLevel::T_STATE_PROCEED2]));
goto *(state);
}
PROCEED2:;
{
state=(((1)==switch_value1) ? (lmap[ ::TopLevel::T_STATE_PROCEED_CASE1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_CASE1]));
goto *(state);
}
PROCEED_CASE1:;
 ::TopLevel::x=(99);
{
state=(lmap[ ::TopLevel::T_STATE_THEN_ELSE_BREAK]);
goto *(state);
}
CASE2:;
{
state=((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ? (lmap[ ::TopLevel::T_STATE_THEN_ELSE_BREAK]) : (lmap[ ::TopLevel::T_STATE_PROCEED3]));
goto *(state);
}
PROCEED3:;
{
state=((!(false)) ? (lmap[ ::TopLevel::T_STATE_THEN]) : (lmap[ ::TopLevel::T_STATE_PROCEED4]));
goto *(state);
}
PROCEED4:;
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
result1=muxtemp;
 ::TopLevel::x=result1;
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
