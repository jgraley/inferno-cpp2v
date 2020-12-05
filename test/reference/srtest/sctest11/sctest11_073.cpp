#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ unsigned int temp_link;
enum TStates
{
T_STATE_ELSE = 11U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK = 1U,
T_STATE_LINK1 = 4U,
T_STATE_LINK2 = 8U,
T_STATE_LINK3 = 10U,
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
unsigned int state;
public:
int x;
int y;
/*temp*/ bool andtemp;
/*temp*/ int f_i;
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
/*temp*/ int muxtemp;
/*temp*/ bool ortemp;
/*temp*/ bool result1;
/*temp*/ int result;
/*temp*/ bool result3;
/*temp*/ int result2;
/*temp*/ int result6;
/*temp*/ int result4;
/*temp*/ int result5;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(4);
 ::TopLevel::andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
 ::TopLevel::state=((! ::TopLevel::andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
continue;
}
if(  ::TopLevel::T_STATE_PROCEED== ::TopLevel::state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::andtemp=( ::TopLevel::y= ::TopLevel::result);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE;
}
if(  ::TopLevel::T_STATE_THEN_ELSE== ::TopLevel::state )
{
 ::TopLevel::result1= ::TopLevel::andtemp;
 ::TopLevel::result1;
 ::TopLevel::ortemp=(!(++ ::TopLevel::x));
 ::TopLevel::state=((! ::TopLevel::ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::T_STATE_THEN== ::TopLevel::state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK1== ::TopLevel::state )
{
 ::TopLevel::result2= ::TopLevel::f_return;
 ::TopLevel::ortemp=( ::TopLevel::y+= ::TopLevel::result2);
 ::TopLevel::state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if(  ::TopLevel::T_STATE_PROCEED_ELSE== ::TopLevel::state )
{
 ::TopLevel::result3= ::TopLevel::ortemp;
 ::TopLevel::result3;
 ::TopLevel::x=(0);
 ::TopLevel::state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if(  ::TopLevel::T_STATE_PROCEED_NEXT== ::TopLevel::state )
{
 ::TopLevel::state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED1);
}
if(  ::TopLevel::T_STATE_PROCEED1== ::TopLevel::state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK2== ::TopLevel::state )
{
 ::TopLevel::result4= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y+= ::TopLevel::result4);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::T_STATE_THEN1== ::TopLevel::state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK3;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK3== ::TopLevel::state )
{
 ::TopLevel::result5= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y-= ::TopLevel::result5);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::T_STATE_ELSE== ::TopLevel::state )
{
 ::TopLevel::result6= ::TopLevel::muxtemp;
 ::TopLevel::result6;
 ::TopLevel::state=(( ::TopLevel::x<(2)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE== ::TopLevel::state )
{
cease(  ::TopLevel::y );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
