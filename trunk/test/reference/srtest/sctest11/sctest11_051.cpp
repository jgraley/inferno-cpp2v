#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int f_return;
void T();
/*temp*/ int f_i;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link1;
int y;
private:
unsigned int link;
public:
enum TStates
{
T_STATE_LINK = 10U,
T_STATE_ELSE = 11U,
T_STATE_PROCEED = 0U,
T_STATE_THEN_ELSE = 2U,
T_STATE_LINK1 = 8U,
T_STATE_PROCEED1 = 7U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_THEN = 9U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK2 = 4U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_THEN1 = 3U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_LINK3 = 1U,
};
int x;
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int muxtemp;
/*temp*/ int result3;
/*temp*/ bool ortemp;
static const unsigned int (lmap[]) = { &&PROCEED, &&LINK, &&THEN_ELSE, &&THEN, &&LINK1, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED1, &&LINK2, &&THEN1, &&LINK3, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ bool result4;
/*temp*/ unsigned int temp_link;
auto unsigned int state;
/*temp*/ int result5;
/*temp*/ bool result6;
/*temp*/ bool andtemp;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
{
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
}
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK3;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK:;
result= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result);
{
state= ::TopLevel::T_STATE_THEN_ELSE;
goto *(lmap[state]);
}
THEN_ELSE:;
result6=andtemp;
result6;
ortemp=(!(++ ::TopLevel::x));
{
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED_ELSE);
goto *(lmap[state]);
}
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK1:;
result1= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result1);
{
state= ::TopLevel::T_STATE_PROCEED_ELSE;
goto *(lmap[state]);
}
PROCEED_ELSE:;
result4=ortemp;
result4;
 ::TopLevel::x=(0);
{
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
}
PROCEED_NEXT:;
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED1);
goto *(lmap[state]);
}
PROCEED1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK2:;
result5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result5);
{
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
}
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK3:;
result2= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result2);
{
state= ::TopLevel::T_STATE_ELSE;
goto *(lmap[state]);
}
ELSE:;
result3=muxtemp;
result3;
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
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(lmap[state]);
}
}