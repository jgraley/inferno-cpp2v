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
/*temp*/ int f_return;
int y;
private:
int i;
unsigned int link;
public:
enum TStates
{
T_STATE_THEN_ELSE = 2U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_ELSE = 11U,
T_STATE_LINK = 1U,
T_STATE_THEN = 3U,
T_STATE_PROCEED = 7U,
T_STATE_PROCEED1 = 0U,
T_STATE_THEN1 = 9U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_LINK1 = 8U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK2 = 4U,
T_STATE_LINK3 = 10U,
};
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link1;
void T();
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool andtemp;
/*temp*/ int result;
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&PROCEED1, &&LINK3, &&THEN_ELSE, &&THEN1, &&LINK2, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED, &&LINK1, &&THEN, &&LINK, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ int result1;
auto unsigned int state;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ bool ortemp;
/*temp*/ int muxtemp;
/*temp*/ int result4;
/*temp*/ bool result5;
/*temp*/ bool result6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED1);
ENTER_f:;
PROCEED_THEN_ELSE:;
ELSE:;
LINK:;
THEN:;
LINK1:;
PROCEED:;
PROCEED_NEXT:;
PROCEED_ELSE:;
LINK2:;
THEN1:;
THEN_ELSE:;
LINK3:;
PROCEED1:;
if(  ::TopLevel::T_STATE_PROCEED1==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK==state )
{
result1= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result1);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result5=andtemp;
result5;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::T_STATE_THEN==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result);
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
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED);
}
if( state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result3);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK3;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK3 )
{
result4= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result4);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result2=muxtemp;
result2;
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
