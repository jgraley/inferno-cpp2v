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
T_STATE_PROCEED_NEXT = 3U,
T_STATE_PROCEED_NEXT1 = 5U,
T_STATE_PROCEED_NEXT2 = 7U,
T_STATE_PROCEED_NEXT3 = 9U,
T_STATE_PROCEED_NEXT4 = 11U,
T_STATE_PROCEED_NEXT_CONTINUE = 1U,
T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK = 0U,
T_STATE_PROCEED_THEN_ELSE = 2U,
T_STATE_PROCEED_THEN_ELSE1 = 4U,
T_STATE_PROCEED_THEN_ELSE2 = 6U,
T_STATE_PROCEED_THEN_ELSE3 = 8U,
T_STATE_PROCEED_THEN_ELSE4 = 10U,
T_STATE_PROCEED_THEN_ELSE5 = 13U,
};
void T();
float fi;
int i;
private:
unsigned int link;
public:
int x;
/*temp*/ unsigned int f_link1;
/*temp*/ unsigned int f_link;
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
for(  ::TopLevel::i=(0); (4)!= ::TopLevel::i;  ::TopLevel::i++ )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>=(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK);
PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK:;
if(  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK==state )
{
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE);
}
PROCEED_NEXT_CONTINUE:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE==state )
{
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
PROCEED_THEN_ELSE:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE==state )
{
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT==state )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1);
}
PROCEED_THEN_ELSE1:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE1==state )
{
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i< ::TopLevel::x)) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE2 :  ::TopLevel::T_STATE_PROCEED_NEXT1);
}
PROCEED_NEXT1:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT1==state )
{
 ::TopLevel::i++;
state=(( ::TopLevel::i< ::TopLevel::x) ?  ::TopLevel::T_STATE_PROCEED_NEXT1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE2);
}
PROCEED_THEN_ELSE2:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE2==state )
{
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=((1)+ ::TopLevel::i) )
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
if(  ::TopLevel::T_STATE_PROCEED_NEXT2==state )
{
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT2 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE3);
}
PROCEED_THEN_ELSE3:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE3==state )
{
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE4 :  ::TopLevel::T_STATE_PROCEED_NEXT3);
}
PROCEED_NEXT3:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT3==state )
{
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT3 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE4);
}
PROCEED_THEN_ELSE4:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE4==state )
{
 ::TopLevel::i=(0);
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE5 :  ::TopLevel::T_STATE_PROCEED_NEXT4);
}
PROCEED_NEXT4:;
if(  ::TopLevel::T_STATE_PROCEED_NEXT4==state )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK:;
if(  ::TopLevel::T_STATE_LINK==state )
{
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT4 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE5);
}
PROCEED_THEN_ELSE5:;
if(  ::TopLevel::T_STATE_PROCEED_THEN_ELSE5==state )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
ENTER_f:;
if(  ::TopLevel::T_STATE_ENTER_f==state )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
