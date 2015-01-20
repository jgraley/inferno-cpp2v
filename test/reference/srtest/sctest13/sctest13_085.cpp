#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
/*temp*/ int f_return;
/*temp*/ int result;
int x;
private:
unsigned int state;
public:
enum TStates
{
T_STATE_ENTER_f = 14U,
T_STATE_PROCEED_NEXT_CONTINUE = 1U,
T_STATE_PROCEED_THEN_ELSE = 10U,
T_STATE_PROCEED_THEN_ELSE_1 = 13U,
T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK = 0U,
T_STATE_PROCEED_NEXT = 11U,
T_STATE_LINK = 12U,
T_STATE_PROCEED_THEN_ELSE_2 = 8U,
T_STATE_PROCEED_NEXT_1 = 7U,
T_STATE_PROCEED_NEXT_2 = 5U,
T_STATE_PROCEED_NEXT_3 = 9U,
T_STATE_PROCEED_THEN_ELSE_3 = 2U,
T_STATE_PROCEED_THEN_ELSE_4 = 6U,
T_STATE_PROCEED_THEN_ELSE_5 = 4U,
T_STATE_PROCEED_NEXT_4 = 3U,
};
int i;
/*temp*/ unsigned int f_link;
private:
unsigned int link;
public:
/*temp*/ unsigned int temp_link;
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<=(4);  ::TopLevel::i++ )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i!=(4);  ::TopLevel::i++ )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>=(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK);
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK) )
{
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3 :  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE);
}
if( enabled&&( ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE== ::TopLevel::state) )
{
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3) )
{
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5 :  ::TopLevel::T_STATE_PROCEED_NEXT_4);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_4) )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_4 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5) )
{
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i< ::TopLevel::x)) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4 :  ::TopLevel::T_STATE_PROCEED_NEXT_2);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_2) )
{
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i< ::TopLevel::x) ?  ::TopLevel::T_STATE_PROCEED_NEXT_2 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4) )
{
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=( ::TopLevel::i+(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2 :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_1) )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2) )
{
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_3);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_3) )
{
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_3 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE) )
{
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT) )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK) )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::x+= ::TopLevel::result;
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1) )
{
cease(  ::TopLevel::x );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f) )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
