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
T_STATE_ELSE = 10,
T_STATE_ENTER_f = 12,
T_STATE_LINK = 0,
T_STATE_LINK_1 = 4,
T_STATE_LINK_2 = 9,
T_STATE_LINK_3 = 11,
T_STATE_PROCEED = 3,
T_STATE_PROCEED_1 = 13,
T_STATE_PROCEED_ELSE = 1,
T_STATE_PROCEED_NEXT = 2,
T_STATE_PROCEED_THEN_ELSE = 5,
T_STATE_THEN = 6,
T_STATE_THEN_1 = 7,
T_STATE_THEN_ELSE = 8,
};
void T();
private:
int i;
public:
int x;
int y;
private:
unsigned int link;
public:
/*temp*/ int f_i;
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&LINK, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED, &&LINK_1, &&PROCEED_THEN_ELSE, &&THEN, &&THEN_1, &&THEN_ELSE, &&LINK_2, &&ELSE, &&LINK_3, &&ENTER_f, &&PROCEED_1 };
auto unsigned int state;
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ bool andtemp;
/*temp*/ bool ortemp;
/*temp*/ bool result_5;
/*temp*/ bool result_6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
{
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_1);
goto *(lmap[state]);
}
PROCEED_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_3;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK_3:;
result= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result);
{
state= ::TopLevel::T_STATE_THEN_ELSE;
goto *(lmap[state]);
}
THEN_ELSE:;
result_5=andtemp;
result_5;
ortemp=(!(++ ::TopLevel::x));
{
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
goto *(lmap[state]);
}
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK:;
result_1= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_1);
{
state= ::TopLevel::T_STATE_PROCEED_ELSE;
goto *(lmap[state]);
}
PROCEED_ELSE:;
result_6=ortemp;
result_6;
 ::TopLevel::x=(0);
{
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
}
PROCEED_NEXT:;
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
}
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK_1:;
result_2= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_2);
{
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
}
THEN_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK_2:;
result_3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_3);
{
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
}
ELSE:;
result_4=muxtemp;
result_4;
{
state=(( ::TopLevel::x<(2)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE:;
cease(  ::TopLevel::y );
return ;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(lmap[state]);
}
}