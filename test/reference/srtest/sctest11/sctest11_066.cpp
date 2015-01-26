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
int x;
private:
unsigned int link;
int i;
public:
/*temp*/ unsigned int f_link;
enum TStates
{
T_STATE_THEN = 9U,
T_STATE_LINK = 8U,
T_STATE_ENTER_f = 13U,
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
};
/*temp*/ int f_i;
/*temp*/ int f_return;
void T();
/*temp*/ unsigned int f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int muxtemp;
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
static const unsigned int (lmap[]) = { &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED, &&ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED };
/*temp*/ bool andtemp;
/*temp*/ int result1;
/*temp*/ bool result2;
/*temp*/ int result3;
auto unsigned int state;
/*temp*/ bool result4;
/*temp*/ bool ortemp;
/*temp*/ int result5;
/*temp*/ int result6;
ENTER_f_PROCEED_THEN_ELSE_ELSE_LINK_THEN_LINK_PROCEED_PROCEED_NEXT_PROCEED_ELSE_LINK_THEN_THEN_ELSE_LINK_PROCEED:;
if( (0U)==(sc_delta_count()) )
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
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK2==state )
{
result5= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result5);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result4=andtemp;
result4;
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
result1= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result1);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if( state== ::TopLevel::T_STATE_PROCEED_ELSE )
{
result2=ortemp;
result2;
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
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result);
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
result6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result6);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result3=muxtemp;
result3;
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
