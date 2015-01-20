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
T_STATE_LINK = 1U,
T_STATE_LINK_1 = 4U,
T_STATE_THEN_ELSE = 2U,
T_STATE_PROCEED = 0U,
T_STATE_PROCEED_1 = 7U,
T_STATE_ENTER_f = 13U,
T_STATE_ELSE = 11U,
T_STATE_THEN = 9U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_LINK_2 = 10U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_LINK_3 = 8U,
T_STATE_THEN_1 = 3U,
};
int x;
/*temp*/ unsigned int f_link;
int y;
private:
int i;
public:
/*temp*/ unsigned int f_link_1;
/*temp*/ int f_i;
/*temp*/ int f_return;
private:
unsigned int link;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ bool andtemp;
/*temp*/ int result_3;
/*temp*/ bool result_4;
auto unsigned int state;
/*temp*/ bool ortemp;
/*temp*/ int muxtemp;
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED };
/*temp*/ bool result_5;
/*temp*/ int result_6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED:;
if( state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK==state )
{
result_3= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_3);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result_5=andtemp;
result_5;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::T_STATE_THEN_1==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_1 )
{
result_2= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_2);
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
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_3;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_3 )
{
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK_2 )
{
result_6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_6);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result_1=muxtemp;
result_1;
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
goto *(lmap[state]);
}
