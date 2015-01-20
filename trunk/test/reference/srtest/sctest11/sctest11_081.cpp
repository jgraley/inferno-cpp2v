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
/*temp*/ int f_i;
/*temp*/ unsigned int f_link;
/*temp*/ int result_1;
/*temp*/ bool result_2;
/*temp*/ int result_3;
/*temp*/ int f_return;
private:
unsigned int link;
int i;
public:
/*temp*/ bool result_4;
/*temp*/ int muxtemp;
enum TStates
{
T_STATE_ELSE = 11U,
T_STATE_ENTER_f = 13U,
T_STATE_PROCEED = 7U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_LINK = 4U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_LINK_1 = 8U,
T_STATE_THEN_ELSE = 2U,
T_STATE_LINK_2 = 1U,
T_STATE_THEN = 9U,
T_STATE_LINK_3 = 10U,
T_STATE_PROCEED_1 = 0U,
T_STATE_THEN_1 = 3U,
T_STATE_PROCEED_ELSE = 5U,
};
private:
unsigned int state;
public:
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ bool ortemp;
/*temp*/ bool andtemp;
void T();
int y;
int x;
/*temp*/ unsigned int temp_link;
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
 ::TopLevel::state=((! ::TopLevel::andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_1);
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_1) )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::T_STATE_LINK_2== ::TopLevel::state) )
{
 ::TopLevel::result_3= ::TopLevel::f_return;
 ::TopLevel::andtemp=( ::TopLevel::y= ::TopLevel::result_3);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_THEN_ELSE) )
{
 ::TopLevel::result_2= ::TopLevel::andtemp;
 ::TopLevel::result_2;
 ::TopLevel::ortemp=(!(++ ::TopLevel::x));
 ::TopLevel::state=((! ::TopLevel::ortemp) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
if( enabled&&( ::TopLevel::T_STATE_THEN_1== ::TopLevel::state) )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK) )
{
 ::TopLevel::result_1= ::TopLevel::f_return;
 ::TopLevel::ortemp=( ::TopLevel::y+= ::TopLevel::result_1);
 ::TopLevel::state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_ELSE== ::TopLevel::state) )
{
 ::TopLevel::result_4= ::TopLevel::ortemp;
 ::TopLevel::result_4;
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
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK_1) )
{
 ::TopLevel::result_5= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y+= ::TopLevel::result_5);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_THEN) )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_3;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK_3) )
{
 ::TopLevel::result_6= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y-= ::TopLevel::result_6);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ELSE) )
{
 ::TopLevel::result= ::TopLevel::muxtemp;
 ::TopLevel::result;
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
