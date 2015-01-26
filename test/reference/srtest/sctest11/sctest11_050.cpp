#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
enum TStates
{
T_STATE_ELSE = 11U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_PROCEED = 0U,
T_STATE_LINK = 4U,
T_STATE_LINK1 = 10U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK2 = 1U,
T_STATE_THEN = 9U,
T_STATE_PROCEED1 = 7U,
T_STATE_LINK3 = 8U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_THEN_ELSE = 2U,
T_STATE_THEN1 = 3U,
};
private:
int i;
public:
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
int y;
/*temp*/ int f_i;
private:
void *link;
public:
/*temp*/ int f_return;
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const void *(lmap[]) = { &&PROCEED, &&LINK, &&THEN_ELSE, &&THEN, &&LINK1, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED1, &&LINK2, &&THEN1, &&LINK3, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ bool andtemp;
/*temp*/ int result;
auto void *state;
/*temp*/ bool ortemp;
/*temp*/ int muxtemp;
/*temp*/ int result1;
/*temp*/ bool result2;
/*temp*/ void *temp_link;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ bool result6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
{
state=((!andtemp) ? (lmap[ ::TopLevel::T_STATE_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK2]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK:;
result1= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result1);
{
state=(lmap[ ::TopLevel::T_STATE_THEN_ELSE]);
goto *(state);
}
THEN_ELSE:;
result2=andtemp;
result2;
ortemp=(!(++ ::TopLevel::x));
{
state=((!ortemp) ? (lmap[ ::TopLevel::T_STATE_THEN1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_ELSE]));
goto *(state);
}
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK1:;
result3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result3);
{
state=(lmap[ ::TopLevel::T_STATE_PROCEED_ELSE]);
goto *(state);
}
PROCEED_ELSE:;
result6=ortemp;
result6;
 ::TopLevel::x=(0);
{
state=((!( ::TopLevel::x<(2))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
{
state=((!( ::TopLevel::x++)) ? (lmap[ ::TopLevel::T_STATE_THEN]) : (lmap[ ::TopLevel::T_STATE_PROCEED1]));
goto *(state);
}
PROCEED1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK3]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK2:;
result4= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result4);
{
state=(lmap[ ::TopLevel::T_STATE_ELSE]);
goto *(state);
}
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK1]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK3:;
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result);
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
