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
enum TStates
{
T_STATE_PROCEED_NEXT = 11U,
T_STATE_PROCEED_NEXT_1 = 5U,
T_STATE_PROCEED_NEXT_2 = 7U,
T_STATE_ENTER_f = 14U,
T_STATE_PROCEED_THEN_ELSE = 6U,
T_STATE_PROCEED_THEN_ELSE_1 = 8U,
T_STATE_PROCEED_NEXT_3 = 9U,
T_STATE_PROCEED_NEXT_4 = 3U,
T_STATE_PROCEED_THEN_ELSE_2 = 4U,
T_STATE_PROCEED_NEXT_CONTINUE = 1U,
T_STATE_PROCEED_THEN_ELSE_3 = 2U,
T_STATE_LINK = 12U,
T_STATE_PROCEED_THEN_ELSE_4 = 10U,
T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK = 0U,
T_STATE_PROCEED_THEN_ELSE_5 = 13U,
};
void T();
/*temp*/ int f_return;
private:
unsigned int link;
public:
int i;
float fi;
int x;
/*temp*/ unsigned int f_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK, &&PROCEED_NEXT_CONTINUE, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT, &&PROCEED_THEN_ELSE_1, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE_2, &&PROCEED_NEXT_2, &&PROCEED_THEN_ELSE_3, &&PROCEED_NEXT_3, &&PROCEED_THEN_ELSE_4, &&PROCEED_NEXT_4, &&LINK, &&PROCEED_THEN_ELSE_5, &&ENTER_f };
auto unsigned int state;
/*temp*/ int result;
/*temp*/ unsigned int temp_link;
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
{
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK);
goto *(lmap[state]);
}
PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3 :  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE);
goto *(lmap[state]);
}
PROCEED_NEXT_CONTINUE:;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE:;
{
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2 :  ::TopLevel::T_STATE_PROCEED_NEXT_4);
goto *(lmap[state]);
}
PROCEED_NEXT:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_4 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE_1:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i< ::TopLevel::x)) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
goto *(lmap[state]);
}
PROCEED_NEXT_1:;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i< ::TopLevel::x) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE_2:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=( ::TopLevel::i+(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT_2);
goto *(lmap[state]);
}
PROCEED_NEXT_2:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
{
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_2 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE_3:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4 :  ::TopLevel::T_STATE_PROCEED_NEXT_3);
goto *(lmap[state]);
}
PROCEED_NEXT_3:;
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_3 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE_4:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5 :  ::TopLevel::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
}
PROCEED_NEXT_4:;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK:;
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE_5:;
cease(  ::TopLevel::x );
return ;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(lmap[state]);
}
}
