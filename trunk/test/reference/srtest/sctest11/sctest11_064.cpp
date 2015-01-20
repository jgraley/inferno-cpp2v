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
/*temp*/ unsigned int f_link;
/*temp*/ int f_i;
/*temp*/ int f_return;
void T();
enum TStates
{
T_STATE_THEN = 9U,
T_STATE_LINK = 4U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_LINK_1 = 1U,
T_STATE_LINK_2 = 8U,
T_STATE_ELSE = 11U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_PROCEED = 0U,
T_STATE_PROCEED_1 = 7U,
T_STATE_THEN_1 = 3U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK_3 = 10U,
T_STATE_THEN_ELSE = 2U,
};
int y;
/*temp*/ unsigned int f_link_1;
int x;
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ bool result_1;
/*temp*/ int result_2;
/*temp*/ int muxtemp;
/*temp*/ bool ortemp;
/*temp*/ int result_3;
static const unsigned int (lmap[]) = { &&PROCEED_1, &&LINK_3, &&THEN_ELSE, &&THEN_1, &&LINK_2, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED, &&LINK_1, &&THEN, &&LINK, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ bool result_4;
auto unsigned int state;
/*temp*/ int result_5;
/*temp*/ unsigned int temp_link;
/*temp*/ int result_6;
/*temp*/ bool andtemp;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
ENTER_f:;
PROCEED_THEN_ELSE:;
ELSE:;
LINK:;
THEN:;
LINK_1:;
PROCEED:;
PROCEED_NEXT:;
PROCEED_ELSE:;
LINK_2:;
THEN_1:;
THEN_ELSE:;
LINK_3:;
PROCEED_1:;
if( state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_1==state )
{
result_2= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_2);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result_4=andtemp;
result_4;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::T_STATE_THEN_1==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result_6= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_6);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if(  ::TopLevel::T_STATE_PROCEED_ELSE==state )
{
result_1=ortemp;
result_1;
 ::TopLevel::x=(0);
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_2 )
{
result_5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_5);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_3;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_3 )
{
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result_3=muxtemp;
result_3;
state=(( ::TopLevel::x<(2)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::TopLevel::y );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
