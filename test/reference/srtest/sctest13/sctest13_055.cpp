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
T_STATE_ENTER_f = 14U,
T_STATE_LINK = 12U,
T_STATE_PROCEED_NEXT1 = 3U,
T_STATE_PROCEED_NEXT = 5U,
T_STATE_PROCEED_NEXT2 = 7U,
T_STATE_PROCEED_NEXT4 = 9U,
T_STATE_PROCEED_NEXT3 = 11U,
T_STATE_PROCEED_NEXT_CONTINUE = 1U,
T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK = 0U,
T_STATE_PROCEED_THEN_ELSE2 = 2U,
T_STATE_PROCEED_THEN_ELSE = 4U,
T_STATE_PROCEED_THEN_ELSE1 = 6U,
T_STATE_PROCEED_THEN_ELSE3 = 8U,
T_STATE_PROCEED_THEN_ELSE5 = 10U,
T_STATE_PROCEED_THEN_ELSE4 = 13U,
};
void T();
float fi;
int i;
private:
unsigned int link;
public:
int x;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link1;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK, &&PROCEED_NEXT_CONTINUE, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT, &&PROCEED_THEN_ELSE1, &&PROCEED_NEXT1, &&PROCEED_THEN_ELSE2, &&PROCEED_NEXT2, &&PROCEED_THEN_ELSE3, &&PROCEED_NEXT3, &&PROCEED_THEN_ELSE4, &&PROCEED_NEXT4, &&LINK, &&PROCEED_THEN_ELSE5, &&ENTER_f };
auto unsigned int state;
/*temp*/ int result;
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
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE2 :  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE);
}
PROCEED_NEXT_CONTINUE:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE )
{
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE2);
}
PROCEED_THEN_ELSE:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE2 )
{
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT1);
}
PROCEED_NEXT:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT1 )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
PROCEED_THEN_ELSE1:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i< ::TopLevel::x)) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT1:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
 ::TopLevel::i++;
state=(( ::TopLevel::i< ::TopLevel::x) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
PROCEED_THEN_ELSE2:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 )
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
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE3 :  ::TopLevel::T_STATE_PROCEED_NEXT2);
}
PROCEED_NEXT2:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT2 )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT2 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE3);
}
PROCEED_THEN_ELSE3:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE3 )
{
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE5 :  ::TopLevel::T_STATE_PROCEED_NEXT4);
}
PROCEED_NEXT3:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT4 )
{
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT4 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE5);
}
PROCEED_THEN_ELSE4:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE5 )
{
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE4 :  ::TopLevel::T_STATE_PROCEED_NEXT3);
}
PROCEED_NEXT4:;
if( state== ::TopLevel::T_STATE_PROCEED_NEXT3 )
{
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK:;
if( state== ::TopLevel::T_STATE_LINK )
{
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT3 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE4);
}
PROCEED_THEN_ELSE5:;
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE4 )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
ENTER_f:;
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
