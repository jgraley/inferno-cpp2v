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
T_STATE_CASE = 3U,
T_STATE_ENTER_f = 15U,
T_STATE_THEN = 11U,
T_STATE_BREAK = 5U,
T_STATE_PROCEED_CASE = 7U,
T_STATE_ELSE = 13U,
T_STATE_THEN_ELSE_BREAK = 14U,
T_STATE_PROCEED_CASE1 = 2U,
T_STATE_PROCEED = 0U,
T_STATE_PROCEED1 = 1U,
T_STATE_PROCEED2 = 9U,
T_STATE_CASE1 = 4U,
T_STATE_PROCEED3 = 6U,
T_STATE_PROCEED4 = 10U,
T_STATE_CASE2 = 8U,
T_STATE_LINK = 12U,
};
void T();
/*temp*/ void *f_link1;
int x;
/*temp*/ int f_return;
private:
void *link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const void *(lmap[]) = { &&PROCEED, &&PROCEED1, &&PROCEED_CASE, &&CASE, &&CASE1, &&BREAK, &&PROCEED2, &&PROCEED_CASE1, &&CASE2, &&PROCEED3, &&PROCEED4, &&THEN, &&LINK, &&ELSE, &&THEN_ELSE_BREAK, &&ENTER_f };
/*temp*/ int result;
/*temp*/ int muxtemp;
auto int switch_value;
auto void *state;
/*temp*/ int result1;
/*temp*/ void *temp_link;
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
{
state=((switch_value==(0)) ? (lmap[ ::TopLevel::T_STATE_CASE1]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED:;
{
state=((switch_value==(4)) ? (lmap[ ::TopLevel::T_STATE_CASE]) : (lmap[ ::TopLevel::T_STATE_PROCEED1]));
goto *(state);
}
PROCEED1:;
{
state=((switch_value==(1)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_CASE1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_CASE1]));
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
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
{
state=(lmap[ ::TopLevel::T_STATE_BREAK]);
goto *(state);
}
BREAK:;
switch_value1=(2);
{
state=((switch_value1==(2)) ? (lmap[ ::TopLevel::T_STATE_CASE2]) : (lmap[ ::TopLevel::T_STATE_PROCEED3]));
goto *(state);
}
PROCEED2:;
{
state=((switch_value1==(1)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_CASE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_CASE]));
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
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (lmap[ ::TopLevel::T_STATE_THEN_ELSE_BREAK]) : (lmap[ ::TopLevel::T_STATE_PROCEED2]));
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
 ::TopLevel::f_link1=(lmap[ ::TopLevel::T_STATE_LINK]);
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
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
