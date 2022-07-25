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
T_STATE_ENTER_f = 14,
T_STATE_LINK = 13,
T_STATE_PROCEED_NEXT = 2,
T_STATE_PROCEED_NEXT_1 = 3,
T_STATE_PROCEED_NEXT_2 = 8,
T_STATE_PROCEED_NEXT_3 = 9,
T_STATE_PROCEED_NEXT_4 = 12,
T_STATE_PROCEED_NEXT_CONTINUE = 7,
T_STATE_PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK = 5,
T_STATE_PROCEED_THEN_ELSE = 0,
T_STATE_PROCEED_THEN_ELSE_1 = 1,
T_STATE_PROCEED_THEN_ELSE_2 = 4,
T_STATE_PROCEED_THEN_ELSE_3 = 6,
T_STATE_PROCEED_THEN_ELSE_4 = 10,
T_STATE_PROCEED_THEN_ELSE_5 = 11,
};
void T();
int i;
int x;
private:
void *link;
public:
float fi;
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
static const void *(lmap[]) = { &&PROCEED_THEN_ELSE, &&PROCEED_THEN_ELSE_1, &&PROCEED_NEXT, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE_2, &&PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK, &&PROCEED_THEN_ELSE_3, &&PROCEED_NEXT_CONTINUE, &&PROCEED_NEXT_2, &&PROCEED_NEXT_3, &&PROCEED_THEN_ELSE_4, &&PROCEED_THEN_ELSE_5, &&PROCEED_NEXT_4, &&LINK, &&ENTER_f };
auto void *state;
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
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK]));
goto *(state);
}
PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE]));
goto *(state);
}
PROCEED_NEXT_CONTINUE:;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_CONTINUE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_3]));
goto *(state);
}
PROCEED_THEN_ELSE_3:;
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_2]));
goto *(state);
}
PROCEED_NEXT_2:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_2]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_2]));
goto *(state);
}
PROCEED_THEN_ELSE_2:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i< ::TopLevel::x)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_3]));
goto *(state);
}
PROCEED_NEXT_3:;
 ::TopLevel::i++;
{
state=(( ::TopLevel::i< ::TopLevel::x) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_3]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_4]));
goto *(state);
}
PROCEED_THEN_ELSE_4:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=((1)+ ::TopLevel::i) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1]));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]));
goto *(state);
}
PROCEED_NEXT_1:;
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
{
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_1]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE_5]));
goto *(state);
}
PROCEED_THEN_ELSE_5:;
 ::TopLevel::i=(0);
{
state=((!( ::TopLevel::i<(4))) ? (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_4]));
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
state=(( ::TopLevel::i<(4)) ? (lmap[ ::TopLevel::T_STATE_PROCEED_NEXT_4]) : (lmap[ ::TopLevel::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE:;
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
