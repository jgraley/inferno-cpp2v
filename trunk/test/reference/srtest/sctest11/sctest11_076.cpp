#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
void T();
/*temp*/ bool andtemp;
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK_1 = 10U,
T_STATE_PROCEED = 0U,
T_STATE_THEN_ELSE = 2U,
T_STATE_PROCEED_THEN_ELSE = 12U,
T_STATE_ELSE = 11U,
T_STATE_PROCEED_NEXT = 6U,
T_STATE_THEN = 3U,
T_STATE_ENTER_f = 13U,
T_STATE_LINK_2 = 8U,
T_STATE_PROCEED_ELSE = 5U,
T_STATE_LINK_3 = 4U,
T_STATE_PROCEED_1 = 7U,
T_STATE_THEN_1 = 9U,
};
int y;
/*temp*/ bool result_1;
/*temp*/ bool result_2;
/*temp*/ int result_3;
private:
unsigned int link;
public:
/*temp*/ unsigned int f_link;
/*temp*/ int f_i;
/*temp*/ int result_4;
/*temp*/ int f_return;
int x;
/*temp*/ int result_5;
/*temp*/ int result_6;
private:
int i;
unsigned int state;
public:
/*temp*/ int muxtemp;
/*temp*/ bool ortemp;
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
 ::TopLevel::state=((! ::TopLevel::andtemp) ?  ::TopLevel::T_STATE_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
enabled=(false);
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::TopLevel::result_5= ::TopLevel::f_return;
 ::TopLevel::andtemp=( ::TopLevel::y= ::TopLevel::result_5);
 ::TopLevel::state= ::TopLevel::T_STATE_THEN_ELSE;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN_ELSE )
{
 ::TopLevel::result_2= ::TopLevel::andtemp;
 ::TopLevel::result_2;
 ::TopLevel::ortemp=(!(++ ::TopLevel::x));
 ::TopLevel::state=((! ::TopLevel::ortemp) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED_ELSE);
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_THEN== ::TopLevel::state )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_3;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK_3 )
{
 ::TopLevel::result_3= ::TopLevel::f_return;
 ::TopLevel::ortemp=( ::TopLevel::y+= ::TopLevel::result_3);
 ::TopLevel::state= ::TopLevel::T_STATE_PROCEED_ELSE;
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_PROCEED_ELSE== ::TopLevel::state )
{
 ::TopLevel::result_1= ::TopLevel::ortemp;
 ::TopLevel::result_1;
 ::TopLevel::x=(0);
 ::TopLevel::state=((!( ::TopLevel::x<(2))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::TopLevel::state=((!( ::TopLevel::x++)) ?  ::TopLevel::T_STATE_THEN_1 :  ::TopLevel::T_STATE_PROCEED_1);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK_2 )
{
 ::TopLevel::result_4= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y+= ::TopLevel::result_4);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_THEN_1 )
{
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK_1 )
{
 ::TopLevel::result_6= ::TopLevel::f_return;
 ::TopLevel::muxtemp=( ::TopLevel::y-= ::TopLevel::result_6);
 ::TopLevel::state= ::TopLevel::T_STATE_ELSE;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_ELSE )
{
 ::TopLevel::result= ::TopLevel::muxtemp;
 ::TopLevel::result;
 ::TopLevel::state=(( ::TopLevel::x<(2)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::TopLevel::y );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
