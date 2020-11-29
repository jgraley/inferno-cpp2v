#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
int i;
float fi;
void T();
private:
void *link;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
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
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK) : (&&PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK));
goto PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK) : (&&PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK));
goto PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK;
PROCEED_PROCEED_NEXT_THEN_ELSE_BREAK:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT_CONTINUE));
goto PROCEED_NEXT_CONTINUE;
PROCEED_NEXT_CONTINUE:;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_CONTINUE) : (&&PROCEED_THEN_ELSE));
goto PROCEED_THEN_ELSE;
PROCEED_THEN_ELSE:;
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_THEN_ELSE1) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE1));
goto PROCEED_THEN_ELSE1;
PROCEED_THEN_ELSE1:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i< ::TopLevel::x)) ? (&&PROCEED_THEN_ELSE2) : (&&PROCEED_NEXT1));
PROCEED_NEXT1:;
 ::TopLevel::i++;
goto *(( ::TopLevel::i< ::TopLevel::x) ? (&&PROCEED_NEXT1) : (&&PROCEED_THEN_ELSE2));
goto PROCEED_THEN_ELSE2;
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
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_THEN_ELSE3) : (&&PROCEED_NEXT2));
PROCEED_NEXT2:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT2) : (&&PROCEED_THEN_ELSE3));
goto PROCEED_THEN_ELSE3;
PROCEED_THEN_ELSE3:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_THEN_ELSE4) : (&&PROCEED_NEXT3));
PROCEED_NEXT3:;
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT3) : (&&PROCEED_THEN_ELSE4));
goto PROCEED_THEN_ELSE4;
PROCEED_THEN_ELSE4:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_THEN_ELSE5) : (&&PROCEED_NEXT4));
PROCEED_NEXT4:;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT4) : (&&PROCEED_THEN_ELSE5));
goto PROCEED_THEN_ELSE5;
PROCEED_THEN_ELSE5:;
cease(  ::TopLevel::x );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
