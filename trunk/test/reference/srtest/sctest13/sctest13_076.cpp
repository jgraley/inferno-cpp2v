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
int i;
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
private:
unsigned int link;
public:
enum TStates
{
T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK = 0U,
T_STATE_PROCEED_NEXT_CONTINUE = 1U,
T_STATE_PROCEED_THEN_ELSE = 2U,
T_STATE_PROCEED_NEXT = 3U,
T_STATE_PROCEED_THEN_ELSE1 = 4U,
T_STATE_PROCEED_NEXT1 = 5U,
T_STATE_PROCEED_THEN_ELSE2 = 6U,
T_STATE_PROCEED_NEXT2 = 7U,
T_STATE_PROCEED_THEN_ELSE3 = 8U,
T_STATE_PROCEED_NEXT3 = 9U,
T_STATE_PROCEED_THEN_ELSE4 = 10U,
T_STATE_PROCEED_NEXT4 = 11U,
T_STATE_LINK = 12U,
T_STATE_PROCEED_THEN_ELSE5 = 13U,
T_STATE_ENTER_f = 14U,
};
private:
unsigned int state;
public:
/*temp*/ unsigned int temp_link;
void T();
/*temp*/ int result;
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
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK )
{
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE )
{
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 )
{
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i< ::TopLevel::x)) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE2 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i< ::TopLevel::x) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE2);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE2 )
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
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE3 :  ::TopLevel::T_STATE_PROCEED_NEXT2);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT2 )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT2 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE3);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE3 )
{
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE4 :  ::TopLevel::T_STATE_PROCEED_NEXT3);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT3 )
{
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT3 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE4);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE4 )
{
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE5 :  ::TopLevel::T_STATE_PROCEED_NEXT4);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT4 )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::x+= ::TopLevel::result;
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT4 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE5);
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE5 )
{
cease(  ::TopLevel::x );
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
 ::TopLevel::f_return=(3);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
