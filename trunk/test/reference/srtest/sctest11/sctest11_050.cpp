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
void T();
enum TStates
{
T_STATE_LINK = 10U,
T_STATE_LINK1 = 4U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_THEN = 3U,
T_STATE_PROCEED = 7U,
T_STATE_ENTER_f = 13U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_ELSE = 11U,
T_STATE_LINK2 = 8U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_THEN_ELSE = 2U,
T_STATE_THEN1 = 9U,
T_STATE_PROCEED1 = 0U,
T_STATE_LINK3 = 1U,
};
/*temp*/ int f_i;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
private:
int i;
void *link;
public:
int y;
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool ortemp;
static const void *(lmap[]) = { &&PROCEED, &&LINK, &&THEN_ELSE, &&THEN, &&LINK1, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED1, &&LINK2, &&THEN1, &&LINK3, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ int result;
/*temp*/ bool result1;
/*temp*/ int result2;
auto void *state;
/*temp*/ int result3;
/*temp*/ bool result4;
/*temp*/ int result5;
/*temp*/ void *temp_link;
/*temp*/ bool andtemp;
/*temp*/ int result6;
/*temp*/ int muxtemp;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
{
state=((!andtemp) ? (lmap[ ::TopLevel::T_STATE_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED1]));
goto *(state);
}
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK3]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK:;
result3= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result3);
{
state=(lmap[ ::TopLevel::T_STATE_THEN_ELSE]);
goto *(state);
}
THEN_ELSE:;
result1=andtemp;
result1;
ortemp=(!(++ ::TopLevel::x));
{
state=((!ortemp) ? (lmap[ ::TopLevel::T_STATE_THEN]) : (lmap[ ::TopLevel::T_STATE_PROCEED_ELSE]));
goto *(state);
}
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK1]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK1:;
result2= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result2);
{
state=(lmap[ ::TopLevel::T_STATE_PROCEED_ELSE]);
goto *(state);
}
PROCEED_ELSE:;
result4=ortemp;
result4;
 ::TopLevel::x=(0);
{
state=((!( ::TopLevel::x<(2))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
{
state=((!( ::TopLevel::x++)) ? (lmap[ ::TopLevel::T_STATE_THEN1]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK2]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK2:;
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result);
{
state=(lmap[ ::TopLevel::T_STATE_ELSE]);
goto *(state);
}
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK3:;
result6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result6);
{
state=(lmap[ ::TopLevel::T_STATE_ELSE]);
goto *(state);
}
ELSE:;
result5=muxtemp;
result5;
{
state=(( ::TopLevel::x<(2)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::TopLevel::y );
return ;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}