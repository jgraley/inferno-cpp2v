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
int y;
void T();
/*temp*/ int f_i;
int x;
enum TStates
{
T_STATE_PROCEED = 0U,
T_STATE_LINK = 8U,
T_STATE_LINK_1 = 1U,
T_STATE_PROCEED_1 = 7U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_ENTER_f = 13U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_LINK_2 = 4U,
T_STATE_ELSE = 11U,
T_STATE_THEN_ELSE = 2U,
T_STATE_THEN = 3U,
T_STATE_LINK_3 = 10U,
T_STATE_THEN_1 = 9U,
};
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link_1;
private:
int i;
public:
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ bool result;
/*temp*/ bool ortemp;
/*temp*/ int result_1;
/*temp*/ bool andtemp;
/*temp*/ int result_2;
/*temp*/ bool result_3;
static const unsigned int (lmap[]) = { &&PROCEED, &&LINK, &&THEN_ELSE, &&THEN, &&LINK_1, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED_1, &&LINK_2, &&THEN_1, &&LINK_3, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ int result_4;
/*temp*/ int muxtemp;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ unsigned int temp_link;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
LINK:;
result_1= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_1);
state= ::TopLevel::T_STATE_THEN_ELSE;
goto *(lmap[state]);
THEN_ELSE:;
result_3=andtemp;
result_3;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
goto *(lmap[state]);
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_2;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
LINK_1:;
result_2= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_2);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
goto *(lmap[state]);
PROCEED_ELSE:;
result=ortemp;
result;
 ::TopLevel::x=(0);
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
PROCEED_NEXT:;
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED_1);
goto *(lmap[state]);
PROCEED_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
LINK_2:;
result_4= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_4);
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
THEN_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_3;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
LINK_3:;
result_6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_6);
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
ELSE:;
result_5=muxtemp;
result_5;
state=(( ::TopLevel::x<(2)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
PROCEED_THEN_ELSE:;
cease(  ::TopLevel::y );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
state=temp_link;
goto *(lmap[state]);
}
