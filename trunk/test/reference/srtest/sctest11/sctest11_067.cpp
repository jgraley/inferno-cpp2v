#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int y;
enum TStates
{
T_STATE_THEN = 9U,
T_STATE_LINK = 8U,
T_STATE_LINK1 = 10U,
T_STATE_PROCEED = 0U,
T_STATE_THEN1 = 3U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_ELSE = 11U,
T_STATE_PROCEED1 = 7U,
T_STATE_LINK2 = 1U,
T_STATE_THEN_ELSE = 2U,
T_STATE_LINK3 = 4U,
T_STATE_ENTER_f = 13U,
};
private:
int i;
public:
/*temp*/ int f_return;
private:
unsigned int link;
public:
/*temp*/ int f_i;
/*temp*/ unsigned int f_link;
void T();
/*temp*/ unsigned int f_link1;
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
auto unsigned int state;
/*temp*/ bool ortemp;
/*temp*/ int result1;
/*temp*/ unsigned int temp_link;
/*temp*/ bool result2;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int muxtemp;
static const unsigned int (lmap[]) = { &&, &&, &&, &&, &&, &&, &&, &&, &&, &&, &&, &&, &&, && };
/*temp*/ bool andtemp;
/*temp*/ int result5;
/*temp*/ bool result6;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK2==state )
{
result= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result2=andtemp;
result2;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::T_STATE_THEN1==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK3;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK3 )
{
result3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result3);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if( state== ::TopLevel::T_STATE_PROCEED_ELSE )
{
result6=ortemp;
result6;
 ::TopLevel::x=(0);
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED1);
}
if( state== ::TopLevel::T_STATE_PROCEED1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result5);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result1);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result4=muxtemp;
result4;
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
}
while( true );
}