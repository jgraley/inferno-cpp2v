#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
int x;
int y;
/*temp*/ int f_return;
/*temp*/ int f_i;
/*temp*/ unsigned int f_link;
private:
int i;
unsigned int link;
public:
enum TStates
{
T_STATE_PROCEED = 0U,
T_STATE_LINK = 1U,
T_STATE_THEN_ELSE = 2U,
T_STATE_THEN = 3U,
T_STATE_LINK1 = 4U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_PROCEED1 = 7U,
T_STATE_LINK2 = 8U,
T_STATE_THEN1 = 9U,
T_STATE_LINK3 = 10U,
T_STATE_ELSE = 11U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_ENTER_f = 13U,
};
private:
unsigned int state;
public:
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int muxtemp;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ bool ortemp;
/*temp*/ bool result4;
/*temp*/ int result5;
/*temp*/ bool andtemp;
void T();
/*temp*/ bool result6;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(4);
 ::TopLevel::andtemp=(++ ::TopLevel::x);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((! ::TopLevel::andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
return ;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::result5= ::TopLevel::f_return;
 ::TopLevel::andtemp=( ::TopLevel::y= ::TopLevel::result5);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN_ELSE )
{
 ::TopLevel::result6= ::TopLevel::andtemp;
 ::TopLevel::result6;
 ::TopLevel::ortemp=(!(++ ::TopLevel::x));
 ::TopLevel::state=((! ::TopLevel::ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
 ::TopLevel::result3= ::TopLevel::f_return;
 ::TopLevel::ortemp=( ::TopLevel::y+= ::TopLevel::result3);
 ::TopLevel::state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_ELSE )
{
 ::TopLevel::result4= ::TopLevel::ortemp;
 ::TopLevel::result4;
 ::TopLevel::x=(0);
 ::TopLevel::state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::TopLevel::state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK2 )
{
 ::TopLevel::result1= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y+= ::TopLevel::result1);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK3;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK3 )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y-= ::TopLevel::result);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ELSE )
{
 ::TopLevel::result2= ::TopLevel::muxtemp;
 ::TopLevel::result2;
 ::TopLevel::state=(( ::TopLevel::x<(2)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::TopLevel::y );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
next_trigger(SC_ZERO_TIME);
}
