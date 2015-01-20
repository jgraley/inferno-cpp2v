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
T_STATE_ENTER_f = 13U,
T_STATE_ELSE = 11U,
T_STATE_THEN = 9U,
T_STATE_PROCEED = 7U,
T_STATE_LINK = 10U,
T_STATE_LINK_1 = 1U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_LINK_2 = 8U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_LINK_3 = 4U,
T_STATE_THEN_1 = 3U,
T_STATE_THEN_ELSE = 2U,
T_STATE_PROCEED_1 = 0U,
};
int x;
/*temp*/ void *f_link_1;
int y;
private:
void *link;
public:
/*temp*/ int f_return;
/*temp*/ int f_i;
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const void *(lmap[]) = { &&PROCEED, &&LINK, &&THEN_ELSE, &&THEN, &&LINK_1, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED_1, &&LINK_2, &&THEN_1, &&LINK_3, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ void *temp_link;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ bool ortemp;
/*temp*/ int muxtemp;
auto void *state;
/*temp*/ bool result_2;
/*temp*/ bool andtemp;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ int result_5;
/*temp*/ bool result_6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
{
state=((!andtemp) ? (lmap[ ::TopLevel::T_STATE_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_1]));
goto *(state);
}
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK_1]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK:;
result_5= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_5);
{
state=(lmap[ ::TopLevel::T_STATE_THEN_ELSE]);
goto *(state);
}
THEN_ELSE:;
result_2=andtemp;
result_2;
ortemp=(!(++ ::TopLevel::x));
{
state=((!ortemp) ? (lmap[ ::TopLevel::T_STATE_THEN_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_ELSE]));
goto *(state);
}
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK_3]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK_1:;
result_3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_3);
{
state=(lmap[ ::TopLevel::T_STATE_PROCEED_ELSE]);
goto *(state);
}
PROCEED_ELSE:;
result_6=ortemp;
result_6;
 ::TopLevel::x=(0);
{
state=((!( ::TopLevel::x<(2))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
{
state=((!( ::TopLevel::x++)) ? (lmap[ ::TopLevel::T_STATE_THEN]) : (lmap[ ::TopLevel::T_STATE_PROCEED]));
goto *(state);
}
PROCEED_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK_2]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK_2:;
result_1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_1);
{
state=(lmap[ ::TopLevel::T_STATE_ELSE]);
goto *(state);
}
THEN_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK_3:;
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result);
{
state=(lmap[ ::TopLevel::T_STATE_ELSE]);
goto *(state);
}
ELSE:;
result_4=muxtemp;
result_4;
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
