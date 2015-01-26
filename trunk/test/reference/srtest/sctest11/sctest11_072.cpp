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
void T();
private:
unsigned int state;
public:
int y;
/*temp*/ int f_i;
private:
int i;
public:
/*temp*/ int f_return;
enum TStates
{
T_STATE_THEN_ELSE = 2U,
T_STATE_LINK = 8U,
T_STATE_ENTER_f = 13U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_PROCEED = 7U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_ELSE = 11U,
T_STATE_LINK1 = 10U,
T_STATE_LINK2 = 1U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_THEN = 9U,
T_STATE_PROCEED1 = 0U,
T_STATE_THEN1 = 3U,
T_STATE_LINK3 = 4U,
};
private:
unsigned int link;
public:
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int muxtemp;
/*temp*/ bool result;
/*temp*/ bool andtemp;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ bool ortemp;
/*temp*/ int result4;
/*temp*/ bool result5;
/*temp*/ unsigned int temp_link;
/*temp*/ int result6;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
 ::TopLevel::state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED1);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED1== ::TopLevel::state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK2 )
{
result3= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result3);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN_ELSE )
{
result=andtemp;
result;
ortemp=(!(++ ::TopLevel::x));
 ::TopLevel::state=((!ortemp) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK3;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK3 )
{
result1= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result1);
 ::TopLevel::state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_ELSE )
{
result5=ortemp;
result5;
 ::TopLevel::x=(0);
 ::TopLevel::state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::TopLevel::state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
result4= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result4);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
result6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result6);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ELSE )
{
result2=muxtemp;
result2;
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
