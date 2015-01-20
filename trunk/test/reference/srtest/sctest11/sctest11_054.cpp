#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
enum TStates
{
T_STATE_PROCEED = 7U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_LINK = 4U,
T_STATE_LINK_1 = 1U,
T_STATE_THEN = 3U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK_2 = 10U,
T_STATE_THEN_ELSE = 2U,
T_STATE_THEN_1 = 9U,
T_STATE_LINK_3 = 8U,
T_STATE_ELSE = 11U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_PROCEED_1 = 0U,
};
/*temp*/ unsigned int f_link;
/*temp*/ int f_i;
void T();
private:
unsigned int link;
public:
/*temp*/ int f_return;
/*temp*/ unsigned int f_link_1;
int y;
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ unsigned int temp_link;
/*temp*/ bool result_2;
/*temp*/ int result_3;
static const unsigned int (lmap[]) = { &&PROCEED, &&LINK, &&THEN_ELSE, &&THEN, &&LINK_1, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED_1, &&LINK_2, &&THEN_1, &&LINK_3, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ int result_4;
auto unsigned int state;
/*temp*/ int muxtemp;
/*temp*/ int result_5;
/*temp*/ bool ortemp;
/*temp*/ bool andtemp;
/*temp*/ bool result_6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_1);
PROCEED:;
if( state== ::TopLevel::T_STATE_PROCEED_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK:;
if(  ::TopLevel::T_STATE_LINK_1==state )
{
result_3= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_3);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
THEN_ELSE:;
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result_2=andtemp;
result_2;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
THEN:;
if(  ::TopLevel::T_STATE_THEN==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK_1:;
if( state== ::TopLevel::T_STATE_LINK )
{
result_4= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_4);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
PROCEED_ELSE:;
if(  ::TopLevel::T_STATE_PROCEED_ELSE==state )
{
result_6=ortemp;
result_6;
 ::TopLevel::x=(0);
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED);
}
PROCEED_1:;
if( state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_3;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK_2:;
if( state== ::TopLevel::T_STATE_LINK_3 )
{
result_1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_1);
state= ::TopLevel::T_STATE_ELSE;
}
THEN_1:;
if( state== ::TopLevel::T_STATE_THEN_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK_3:;
if( state== ::TopLevel::T_STATE_LINK_2 )
{
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result);
state= ::TopLevel::T_STATE_ELSE;
}
ELSE:;
if( state== ::TopLevel::T_STATE_ELSE )
{
result_5=muxtemp;
result_5;
state=(( ::TopLevel::x<(2)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
PROCEED_THEN_ELSE:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::TopLevel::y );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
ENTER_f:;
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
