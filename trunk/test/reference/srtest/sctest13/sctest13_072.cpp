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
int x;
int i;
private:
unsigned int state;
public:
void T();
enum TStates
{
T_STATE_LINK = 12U,
T_STATE_PROCEED_THEN_ELSE = 8U,
T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK = 0U,
T_STATE_PROCEED_THEN_ELSE_1 = 6U,
T_STATE_PROCEED_NEXT = 7U,
T_STATE_PROCEED_NEXT_CONTINUE = 1U,
T_STATE_PROCEED_NEXT_1 = 11U,
T_STATE_PROCEED_THEN_ELSE_2 = 4U,
T_STATE_PROCEED_THEN_ELSE_3 = 2U,
T_STATE_PROCEED_NEXT_2 = 3U,
T_STATE_PROCEED_NEXT_3 = 9U,
T_STATE_PROCEED_THEN_ELSE_4 = 10U,
T_STATE_PROCEED_THEN_ELSE_5 = 13U,
T_STATE_ENTER_f = 14U,
T_STATE_PROCEED_NEXT_4 = 5U,
};
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
do
{
if( (0U)==(sc_delta_count()) )
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
wait(SC_ZERO_TIME);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK);
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK )
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
if(  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE== ::TopLevel::state )
{
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3 )
{
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2 :  ::TopLevel::T_STATE_PROCEED_NEXT_2);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_2 )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_2 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2 )
{
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i< ::TopLevel::x)) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT_4);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_4 )
{
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i< ::TopLevel::x) ?  ::TopLevel::T_STATE_PROCEED_NEXT_4 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 )
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
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4 :  ::TopLevel::T_STATE_PROCEED_NEXT_3);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_3 )
{
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_3 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4 )
{
 ::TopLevel::i=(0);
 ::TopLevel::state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5 :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_NEXT_1 )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
 ::TopLevel::state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5);
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5 )
{
cease(  ::TopLevel::x );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
 ::TopLevel::state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
