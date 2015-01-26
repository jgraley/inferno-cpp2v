#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
/*temp*/ int result;
/*temp*/ bool ortemp;
private:
unsigned int state;
public:
void T();
/*temp*/ int result1;
enum TStates
{
T_STATE_ELSE = 11U,
T_STATE_LINK = 8U,
T_STATE_PROCEED = 7U,
T_STATE_LINK1 = 10U,
T_STATE_THEN_ELSE = 2U,
T_STATE_ENTER_f = 13U,
T_STATE_THEN = 9U,
T_STATE_PROCEED1 = 0U,
T_STATE_LINK2 = 1U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_LINK3 = 4U,
T_STATE_THEN1 = 3U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_PROCEED_NEXT = 6U,
};
/*temp*/ int muxtemp;
private:
int i;
public:
/*temp*/ int result2;
/*temp*/ bool result3;
int x;
/*temp*/ int result4;
/*temp*/ bool andtemp;
int y;
/*temp*/ bool result5;
/*temp*/ unsigned int temp_link;
/*temp*/ int result6;
/*temp*/ unsigned int f_link;
/*temp*/ int f_i;
/*temp*/ int f_return;
private:
unsigned int link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(4);
 ::TopLevel::andtemp=(++ ::TopLevel::x);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((! ::TopLevel::andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED1);
enabled=(false);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED1== ::TopLevel::state) )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK2) )
{
 ::TopLevel::result2= ::TopLevel::f_return;
 ::TopLevel::andtemp=( ::TopLevel::y= ::TopLevel::result2);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_THEN_ELSE) )
{
 ::TopLevel::result3= ::TopLevel::andtemp;
 ::TopLevel::result3;
 ::TopLevel::ortemp=(!(++ ::TopLevel::x));
 ::TopLevel::state=((! ::TopLevel::ortemp) ?  ::TopLevel::T_STATE_THEN1 :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_THEN1) )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK3;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK3) )
{
 ::TopLevel::result1= ::TopLevel::f_return;
 ::TopLevel::ortemp=( ::TopLevel::y+= ::TopLevel::result1);
 ::TopLevel::state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_ELSE) )
{
 ::TopLevel::result5= ::TopLevel::ortemp;
 ::TopLevel::result5;
 ::TopLevel::x=(0);
 ::TopLevel::state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT) )
{
 ::TopLevel::state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED) )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK) )
{
 ::TopLevel::result4= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y+= ::TopLevel::result4);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_THEN) )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK1) )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y-= ::TopLevel::result);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ELSE) )
{
 ::TopLevel::result6= ::TopLevel::muxtemp;
 ::TopLevel::result6;
 ::TopLevel::state=(( ::TopLevel::x<(2)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE) )
{
cease(  ::TopLevel::y );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f) )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
