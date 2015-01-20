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
private:
void *link;
public:
/*temp*/ void *f_link;
enum TStates
{
T_STATE_PROCEED_NEXT = 5U,
T_STATE_ENTER_f = 14U,
T_STATE_LINK = 12U,
T_STATE_PROCEED_THEN_ELSE = 6U,
T_STATE_PROCEED_THEN_ELSE_1 = 13U,
T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK = 0U,
T_STATE_PROCEED_THEN_ELSE_2 = 10U,
T_STATE_PROCEED_THEN_ELSE_3 = 4U,
T_STATE_PROCEED_NEXT_1 = 3U,
T_STATE_PROCEED_NEXT_2 = 11U,
T_STATE_PROCEED_NEXT_CONTINUE = 1U,
T_STATE_PROCEED_THEN_ELSE_4 = 2U,
T_STATE_PROCEED_NEXT_3 = 9U,
T_STATE_PROCEED_NEXT_4 = 7U,
T_STATE_PROCEED_THEN_ELSE_5 = 8U,
};
/*temp*/ int f_return;
int x;
float fi;
int i;
/*temp*/ void *f_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const void *(lmap[]) = { &&PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK, &&PROCEED_NEXT_CONTINUE, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT, &&PROCEED_THEN_ELSE_1, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE_2, &&PROCEED_NEXT_2, &&PROCEED_THEN_ELSE_3, &&PROCEED_NEXT_3, &&PROCEED_THEN_ELSE_4, &&PROCEED_NEXT_4, &&LINK, &&PROCEED_THEN_ELSE_5, &&ENTER_f };
auto void *state;
/*temp*/ int result;
/*temp*/ void *temp_link;
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
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK]) : (lmap[ ::TopLevel::T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK]));
goto *(state);
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
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE]));
goto *(state);
}
PROCEED_NEXT_CONTINUE:;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4]));
goto *(state);
}
PROCEED_THEN_ELSE:;
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]));
goto *(state);
}
PROCEED_NEXT:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3]));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i< ::TopLevel::x)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT_1:;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i< ::TopLevel::x) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
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
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_4]));
goto *(state);
}
PROCEED_NEXT_2:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_4]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5]));
goto *(state);
}
PROCEED_THEN_ELSE_3:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_3]));
goto *(state);
}
PROCEED_NEXT_3:;
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_3]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2]));
goto *(state);
}
PROCEED_THEN_ELSE_4:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_2]));
goto *(state);
}
PROCEED_NEXT_4:;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK:;
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_2]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]));
goto *(state);
}
PROCEED_THEN_ELSE_5:;
cease(  ::TopLevel::x );
return ;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
