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
enum TStates
{
T_STATE_ENTER_f = 14U,
T_STATE_PROCEED_THEN_ELSE = 10U,
T_STATE_PROCEED_NEXT = 9U,
T_STATE_PROCEED_NEXT1 = 11U,
T_STATE_PROCEED_NEXT_CONTINUE = 1U,
T_STATE_LINK = 12U,
T_STATE_PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK = 0U,
T_STATE_PROCEED_THEN_ELSE1 = 8U,
T_STATE_PROCEED_THEN_ELSE2 = 2U,
T_STATE_PROCEED_THEN_ELSE3 = 13U,
T_STATE_PROCEED_THEN_ELSE4 = 4U,
T_STATE_PROCEED_NEXT2 = 7U,
T_STATE_PROCEED_NEXT3 = 3U,
T_STATE_PROCEED_NEXT4 = 5U,
T_STATE_PROCEED_THEN_ELSE5 = 6U,
};
int i;
int x;
/*temp*/ void *f_link;
private:
void *link;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link1;
float fi;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
auto void *state;
/*temp*/ void *temp_link;
static const void *(lmap[]) = { &&PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK, &&PROCEED_NEXT_CONTINUE, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT, &&PROCEED_THEN_ELSE1, &&PROCEED_NEXT1, &&PROCEED_THEN_ELSE2, &&PROCEED_NEXT2, &&PROCEED_THEN_ELSE3, &&PROCEED_NEXT3, &&PROCEED_THEN_ELSE4, &&PROCEED_NEXT4, &&LINK, &&PROCEED_THEN_ELSE5, &&ENTER_f };
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
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE2]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE]));
goto *(state);
}
PROCEED_NEXT_CONTINUE:;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE2]));
goto *(state);
}
PROCEED_THEN_ELSE:;
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE4]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT3]));
goto *(state);
}
PROCEED_NEXT:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT3]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE4]));
goto *(state);
}
PROCEED_THEN_ELSE1:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i< ::TopLevel::x)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE5]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT4]));
goto *(state);
}
PROCEED_NEXT1:;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i< ::TopLevel::x) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT4]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE5]));
goto *(state);
}
PROCEED_THEN_ELSE2:;
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
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT2]));
goto *(state);
}
PROCEED_NEXT2:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT2]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE1]));
goto *(state);
}
PROCEED_THEN_ELSE3:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT3:;
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE4:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE3]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT1]));
goto *(state);
}
PROCEED_NEXT4:;
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
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE3]));
goto *(state);
}
PROCEED_THEN_ELSE5:;
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