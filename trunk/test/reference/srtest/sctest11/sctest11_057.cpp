#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
private:
int i;
public:
/*temp*/ unsigned int f_link1;
int y;
private:
unsigned int link;
public:
/*temp*/ int f_i;
int x;
void T();
enum TStates
{
T_STATE_PROCEED_ELSE = 5U,
T_STATE_ELSE = 11U,
T_STATE_LINK = 1U,
T_STATE_THEN = 3U,
T_STATE_PROCEED = 7U,
T_STATE_PROCEED1 = 0U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_LINK1 = 8U,
T_STATE_ENTER_f = 13U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_THEN1 = 9U,
T_STATE_LINK2 = 4U,
T_STATE_THEN_ELSE = 2U,
T_STATE_LINK3 = 10U,
};
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ bool ortemp;
/*temp*/ int muxtemp;
/*temp*/ int result1;
/*temp*/ bool result2;
/*temp*/ bool result3;
/*temp*/ unsigned int temp_link;
/*temp*/ int result4;
static const unsigned int (lmap[]) = { &&PROCEED, &&LINK, &&THEN_ELSE, &&THEN, &&LINK1, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED1, &&LINK2, &&THEN1, &&LINK3, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ int result5;
auto unsigned int state;
/*temp*/ bool andtemp;
/*temp*/ int result6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED1);
PROCEED:;
if(  ::TopLevel::T_STATE_PROCEED1==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK:;
if(  ::TopLevel::T_STATE_LINK==state )
{
result= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
THEN_ELSE:;
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result3=andtemp;
result3;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
THEN:;
if(  ::TopLevel::T_STATE_THEN==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK1:;
if( state== ::TopLevel::T_STATE_LINK2 )
{
result6= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result6);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
PROCEED_ELSE:;
if( state== ::TopLevel::T_STATE_PROCEED_ELSE )
{
result2=ortemp;
result2;
 ::TopLevel::x=(0);
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED);
}
PROCEED1:;
if( state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK2:;
if( state== ::TopLevel::T_STATE_LINK1 )
{
result5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result5);
state= ::TopLevel::T_STATE_ELSE;
}
THEN1:;
if( state== ::TopLevel::T_STATE_THEN1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK3;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK3:;
if( state== ::TopLevel::T_STATE_LINK3 )
{
result4= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result4);
state= ::TopLevel::T_STATE_ELSE;
}
ELSE:;
if( state== ::TopLevel::T_STATE_ELSE )
{
result1=muxtemp;
result1;
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
