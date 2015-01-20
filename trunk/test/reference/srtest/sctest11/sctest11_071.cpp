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
enum TStates
{
T_STATE_ENTER_f = 13U,
T_STATE_PROCEED = 7U,
T_STATE_THEN = 3U,
T_STATE_PROCEED_1 = 0U,
T_STATE_LINK = 4U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_THEN_1 = 9U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_LINK_1 = 1U,
T_STATE_LINK_2 = 8U,
T_STATE_THEN_ELSE = 2U,
T_STATE_LINK_3 = 10U,
T_STATE_ELSE = 11U,
};
private:
int i;
public:
int x;
private:
unsigned int link;
public:
/*temp*/ unsigned int f_link;
void T();
int y;
/*temp*/ int f_i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
auto unsigned int state;
/*temp*/ int muxtemp;
/*temp*/ bool andtemp;
/*temp*/ int result_1;
/*temp*/ unsigned int temp_link;
/*temp*/ int result_2;
/*temp*/ bool ortemp;
/*temp*/ int result_3;
/*temp*/ bool result_4;
/*temp*/ int result_5;
/*temp*/ bool result_6;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_1);
continue;
}
if( state== ::TopLevel::T_STATE_PROCEED_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_1==state )
{
result_1= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_1);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result_6=andtemp;
result_6;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::T_STATE_THEN==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if(  ::TopLevel::T_STATE_PROCEED_ELSE==state )
{
result_4=ortemp;
result_4;
 ::TopLevel::x=(0);
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED);
}
if( state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_2 )
{
result_3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_3);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_3;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_3 )
{
result_2= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_2);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result_5=muxtemp;
result_5;
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
wait(SC_ZERO_TIME);
}
while( true );
}
