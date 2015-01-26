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
private:
unsigned int link;
public:
int y;
void T();
/*temp*/ int f_return;
/*temp*/ int f_i;
int x;
/*temp*/ unsigned int f_link1;
enum TStates
{
T_STATE_ENTER_f = 13U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_PROCEED = 7U,
T_STATE_LINK = 8U,
T_STATE_THEN_ELSE = 2U,
T_STATE_LINK1 = 4U,
T_STATE_LINK2 = 10U,
T_STATE_ELSE = 11U,
T_STATE_PROCEED1 = 0U,
T_STATE_THEN = 3U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_THEN1 = 9U,
T_STATE_LINK3 = 1U,
};
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ bool andtemp;
static const unsigned int (lmap[]) = { &&PROCEED, &&LINK, &&THEN_ELSE, &&THEN, &&LINK1, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED1, &&LINK2, &&THEN1, &&LINK3, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
/*temp*/ int result1;
/*temp*/ bool result2;
/*temp*/ int result3;
/*temp*/ bool ortemp;
/*temp*/ int result4;
/*temp*/ bool result5;
/*temp*/ int muxtemp;
auto unsigned int state;
/*temp*/ unsigned int temp_link;
/*temp*/ int result6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED1);
PROCEED:;
if(  ::TopLevel::T_STATE_PROCEED1==state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK3;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK:;
if( state== ::TopLevel::T_STATE_LINK3 )
{
result= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
THEN_ELSE:;
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result5=andtemp;
result5;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
THEN:;
if( state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK1:;
if( state== ::TopLevel::T_STATE_LINK1 )
{
result4= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result4);
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
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED);
}
PROCEED1:;
if( state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK2:;
if( state== ::TopLevel::T_STATE_LINK )
{
result3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result3);
state= ::TopLevel::T_STATE_ELSE;
}
THEN1:;
if( state== ::TopLevel::T_STATE_THEN1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK3:;
if( state== ::TopLevel::T_STATE_LINK2 )
{
result6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result6);
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
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
