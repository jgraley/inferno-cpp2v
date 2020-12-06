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
int i;
int x;
private:
void *link;
public:
float fi;
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
for(  ::TopLevel::i=(0); (4)!= ::TopLevel::i;  ::TopLevel::i++ )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>=(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE_BREAK) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
goto THEN_ELSE_BREAK;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED));
PROCEED:;
goto THEN_ELSE_BREAK;
THEN_ELSE_BREAK:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE) : (&&PROCEED_NEXT1));
PROCEED_NEXT1:;
goto CONTINUE;
CONTINUE:;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT1) : (&&PROCEED1));
PROCEED1:;
goto THEN_ELSE;
THEN_ELSE:;
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE1) : (&&PROCEED_NEXT2));
PROCEED_NEXT2:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT2) : (&&PROCEED2));
PROCEED2:;
goto THEN_ELSE1;
THEN_ELSE1:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i< ::TopLevel::x)) ? (&&THEN_ELSE2) : (&&PROCEED_NEXT3));
PROCEED_NEXT3:;
 ::TopLevel::i++;
goto *(( ::TopLevel::i< ::TopLevel::x) ? (&&PROCEED_NEXT3) : (&&PROCEED3));
PROCEED3:;
goto THEN_ELSE2;
THEN_ELSE2:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=((1)+ ::TopLevel::i) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE3) : (&&PROCEED_NEXT4));
PROCEED_NEXT4:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT4) : (&&PROCEED4));
PROCEED4:;
goto THEN_ELSE3;
THEN_ELSE3:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE4) : (&&PROCEED_NEXT5));
PROCEED_NEXT5:;
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT5) : (&&PROCEED5));
PROCEED5:;
goto THEN_ELSE4;
THEN_ELSE4:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE5) : (&&PROCEED_NEXT6));
PROCEED_NEXT6:;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT6) : (&&PROCEED6));
PROCEED6:;
goto THEN_ELSE5;
THEN_ELSE5:;
cease(  ::TopLevel::x );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
