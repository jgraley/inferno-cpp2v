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
/*temp*/ void *f_link_1;
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
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE) : (&&PROCEED_NEXT_1));
PROCEED_NEXT_1:;
goto CONTINUE;
CONTINUE:;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_1) : (&&PROCEED_1));
PROCEED_1:;
goto THEN_ELSE;
THEN_ELSE:;
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE_1) : (&&PROCEED_NEXT_2));
PROCEED_NEXT_2:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_2) : (&&PROCEED_2));
PROCEED_2:;
goto THEN_ELSE_1;
THEN_ELSE_1:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i< ::TopLevel::x)) ? (&&THEN_ELSE_2) : (&&PROCEED_NEXT_3));
PROCEED_NEXT_3:;
 ::TopLevel::i++;
goto *(( ::TopLevel::i< ::TopLevel::x) ? (&&PROCEED_NEXT_3) : (&&PROCEED_3));
PROCEED_3:;
goto THEN_ELSE_2;
THEN_ELSE_2:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=((1)+ ::TopLevel::i) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE_3) : (&&PROCEED_NEXT_4));
PROCEED_NEXT_4:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_4) : (&&PROCEED_4));
PROCEED_4:;
goto THEN_ELSE_3;
THEN_ELSE_3:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE_4) : (&&PROCEED_NEXT_5));
PROCEED_NEXT_5:;
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_5) : (&&PROCEED_5));
PROCEED_5:;
goto THEN_ELSE_4;
THEN_ELSE_4:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_ELSE_5) : (&&PROCEED_NEXT_6));
PROCEED_NEXT_6:;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_6) : (&&PROCEED_6));
PROCEED_6:;
goto THEN_ELSE_5;
THEN_ELSE_5:;
cease(  ::TopLevel::x );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
