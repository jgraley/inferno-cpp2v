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
T_STATE_THEN_ELSE = 2U,
T_STATE_LINK = 1U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_ELSE = 11U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_LINK_1 = 8U,
T_STATE_THEN = 3U,
T_STATE_THEN_1 = 9U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK_2 = 4U,
T_STATE_PROCEED = 0U,
T_STATE_LINK_3 = 10U,
T_STATE_PROCEED_1 = 7U,
};
/*temp*/ int f_return;
private:
unsigned int link;
public:
int x;
/*temp*/ unsigned int f_link;
int y;
private:
int i;
public:
/*temp*/ unsigned int f_link_1;
/*temp*/ int f_i;
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ bool andtemp;
/*temp*/ int result_1;
auto unsigned int state;
/*temp*/ bool result_2;
static const unsigned int (lmap[]) = { &&, &&, &&, &&, &&, &&, &&, &&, &&, &&, &&, &&, &&, && };
/*temp*/ bool result_3;
/*temp*/ int result_4;
/*temp*/ int muxtemp;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ bool ortemp;
/*temp*/ unsigned int temp_link;
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
if( state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK==state )
{
result_4= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_4);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result_2=andtemp;
result_2;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::T_STATE_THEN==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_2 )
{
result= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if(  ::TopLevel::T_STATE_PROCEED_ELSE==state )
{
result_3=ortemp;
result_3;
 ::TopLevel::x=(0);
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_1 )
{
result_1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_1);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_3;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_3 )
{
result_6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_6);
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
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
