#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int f_i;
private:
int i;
public:
/*temp*/ unsigned int f_link;
void T();
int x;
/*temp*/ unsigned int f_link1;
int y;
/*temp*/ int f_return;
private:
unsigned int link;
public:
enum TStates
{
T_STATE_THEN = 9U,
T_STATE_LINK = 4U,
T_STATE_ENTER_f = 13U,
T_STATE_PROCEED = 0U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_THEN_ELSE = 2U,
T_STATE_ELSE = 11U,
T_STATE_LINK1 = 10U,
T_STATE_PROCEED1 = 7U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_LINK2 = 8U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_LINK3 = 1U,
T_STATE_THEN1 = 3U,
};
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ int result;
/*temp*/ unsigned int temp_link;
/*temp*/ int result1;
/*temp*/ bool ortemp;
/*temp*/ bool result2;
/*temp*/ bool result3;
static const unsigned int (lmap[]) = { &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED };
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ bool andtemp;
/*temp*/ int result6;
/*temp*/ int muxtemp;
ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED:;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
goto *(lmap[state]);
}
if(  ::TopLevel::T_STATE_PROCEED==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK3;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK3 )
{
result1= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result1);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result3=andtemp;
result3;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if( state== ::TopLevel::T_STATE_THEN1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if( state== ::TopLevel::T_STATE_PROCEED_ELSE )
{
result2=ortemp;
result2;
 ::TopLevel::x=(0);
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED1);
}
if( state== ::TopLevel::T_STATE_PROCEED1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result4= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result4);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result5);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result6=muxtemp;
result6;
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
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
