#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
/*temp*/ int f_return;
int x;
/*temp*/ unsigned int f_link;
enum TStates
{
T_STATE_LINK = 12U,
T_STATE_PROCEED_THEN_ELSE = 4U,
T_STATE_PROCEED_THEN_ELSE_1 = 13U,
T_STATE_PROCEED_NEXT = 9U,
T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK = 0U,
T_STATE_PROCEED_THEN_ELSE_2 = 2U,
T_STATE_PROCEED_NEXT_CONTINUE = 1U,
T_STATE_ENTER_f = 14U,
T_STATE_PROCEED_THEN_ELSE_3 = 6U,
T_STATE_PROCEED_NEXT_1 = 11U,
T_STATE_PROCEED_NEXT_2 = 3U,
T_STATE_PROCEED_THEN_ELSE_4 = 8U,
T_STATE_PROCEED_NEXT_3 = 7U,
T_STATE_PROCEED_THEN_ELSE_5 = 10U,
T_STATE_PROCEED_NEXT_4 = 5U,
};
float fi;
int i;
/*temp*/ unsigned int f_link_1;
private:
unsigned int link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
static const unsigned int (lmap[]) = { &&PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK, &&PROCEED_NEXT_CONTINUE, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT, &&PROCEED_THEN_ELSE_1, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE_2, &&PROCEED_NEXT_2, &&PROCEED_THEN_ELSE_3, &&PROCEED_NEXT_3, &&PROCEED_THEN_ELSE_4, &&PROCEED_NEXT_4, &&LINK, &&PROCEED_THEN_ELSE_5, &&ENTER_f };
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
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK);
PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK:;
if( state== ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK )
{
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2 :  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE);
}
PROCEED_NEXT_CONTINUE:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE==state )
{
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2);
}
PROCEED_THEN_ELSE:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2 )
{
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_2);
}
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT_2 )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_2 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
PROCEED_THEN_ELSE_1:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i< ::TopLevel::x)) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3 :  ::TopLevel::T_STATE_PROCEED_NEXT_4);
}
PROCEED_NEXT_1:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT_4 )
{
 ::TopLevel::i++;
state=(( ::TopLevel::i< ::TopLevel::x) ?  ::TopLevel::T_STATE_PROCEED_NEXT_4 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3);
}
PROCEED_THEN_ELSE_2:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3 )
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
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4 :  ::TopLevel::T_STATE_PROCEED_NEXT_3);
}
PROCEED_NEXT_2:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT_3 )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_3 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4);
}
PROCEED_THEN_ELSE_3:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4 )
{
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT_3:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5);
}
PROCEED_THEN_ELSE_4:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5 )
{
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
}
PROCEED_NEXT_4:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT_1 )
{
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK:;
if( state== ::TopLevel::T_STATE_LINK )
{
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
PROCEED_THEN_ELSE_5:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
ENTER_f:;
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
