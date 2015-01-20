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
private:
unsigned int state;
public:
int x;
/*temp*/ int f_return;
enum TStates
{
T_STATE_PROCEED_NEXT = 6U,
T_STATE_THEN = 3U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK = 10U,
T_STATE_ELSE = 11U,
T_STATE_LINK_1 = 4U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_LINK_2 = 1U,
T_STATE_THEN_1 = 9U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_LINK_3 = 8U,
T_STATE_THEN_ELSE = 2U,
T_STATE_PROCEED = 0U,
T_STATE_PROCEED_1 = 7U,
};
private:
int i;
public:
/*temp*/ int f_i;
private:
unsigned int link;
public:
int y;
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ bool ortemp;
/*temp*/ int result_1;
/*temp*/ bool result_2;
/*temp*/ int result_3;
/*temp*/ bool result_4;
/*temp*/ int muxtemp;
/*temp*/ int result_5;
/*temp*/ unsigned int temp_link;
/*temp*/ int result_6;
/*temp*/ bool andtemp;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
 ::TopLevel::state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_2== ::TopLevel::state )
{
result_5= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_5);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN_ELSE )
{
result_4=andtemp;
result_4;
ortemp=(!(++ ::TopLevel::x));
 ::TopLevel::state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::T_STATE_THEN== ::TopLevel::state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK_1 )
{
result= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result);
 ::TopLevel::state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if(  ::TopLevel::T_STATE_PROCEED_ELSE== ::TopLevel::state )
{
result_2=ortemp;
result_2;
 ::TopLevel::x=(0);
 ::TopLevel::state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::TopLevel::state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED_1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_3;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK_3 )
{
result_1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_1);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
result_6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_6);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ELSE )
{
result_3=muxtemp;
result_3;
 ::TopLevel::state=(( ::TopLevel::x<(2)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::TopLevel::y );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
 ::TopLevel::state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
