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
T_STATE_ELSE = 11U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK = 1U,
T_STATE_LINK1 = 4U,
T_STATE_LINK3 = 8U,
T_STATE_LINK2 = 10U,
T_STATE_PROCEED = 0U,
T_STATE_PROCEED1 = 7U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_THEN = 3U,
T_STATE_THEN1 = 9U,
T_STATE_THEN_ELSE = 2U,
};
void T();
private:
int i;
unsigned int link;
public:
int x;
int y;
/*temp*/ int f_i;
/*temp*/ unsigned int f_link1;
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&PROCEED, &&LINK, &&THEN_ELSE, &&THEN, &&LINK1, &&PROCEED_ELSE, &&PROCEED_NEXT, &&PROCEED1, &&LINK2, &&THEN1, &&LINK3, &&ELSE, &&PROCEED_THEN_ELSE, &&ENTER_f };
auto unsigned int state;
/*temp*/ bool andtemp;
/*temp*/ int muxtemp;
/*temp*/ bool ortemp;
/*temp*/ bool result5;
/*temp*/ bool result1;
/*temp*/ int result6;
/*temp*/ int result4;
/*temp*/ int result3;
/*temp*/ int result;
/*temp*/ int result2;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
state=((!andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
PROCEED:;
if( state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK:;
if( state== ::TopLevel::T_STATE_LINK )
{
result6= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result6);
state= ::TopLevel::T_STATE_THEN_ELSE;
}
THEN_ELSE:;
if( state== ::TopLevel::T_STATE_THEN_ELSE )
{
result1=andtemp;
result1;
ortemp=(!(++ ::TopLevel::x));
state=((!ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
THEN:;
if( state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK1:;
if( state== ::TopLevel::T_STATE_LINK1 )
{
result3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result3);
state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
PROCEED_ELSE:;
if( state== ::TopLevel::T_STATE_PROCEED_ELSE )
{
result5=ortemp;
result5;
 ::TopLevel::x=(0);
state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED1);
}
PROCEED1:;
if( state== ::TopLevel::T_STATE_PROCEED1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK3;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK2:;
if( state== ::TopLevel::T_STATE_LINK3 )
{
result2= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result2);
state= ::TopLevel::T_STATE_ELSE;
}
THEN1:;
if( state== ::TopLevel::T_STATE_THEN1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK3:;
if( state== ::TopLevel::T_STATE_LINK2 )
{
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result);
state= ::TopLevel::T_STATE_ELSE;
}
ELSE:;
if( state== ::TopLevel::T_STATE_ELSE )
{
result4=muxtemp;
result4;
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
