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
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK) : (&&PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK));
goto PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK) : (&&PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK));
goto PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK;
PROCEED_NEXT_PROCEED_THEN_ELSE_BREAK:;
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
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_THEN_ELSE_1) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE_1));
goto PROCEED_THEN_ELSE_1;
PROCEED_THEN_ELSE_1:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i< ::TopLevel::x)) ? (&&PROCEED_THEN_ELSE_2) : (&&PROCEED_NEXT_1));
PROCEED_NEXT_1:;
 ::TopLevel::i++;
goto *(( ::TopLevel::i< ::TopLevel::x) ? (&&PROCEED_NEXT_1) : (&&PROCEED_THEN_ELSE_2));
goto PROCEED_THEN_ELSE_2;
PROCEED_THEN_ELSE_2:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=((1)+ ::TopLevel::i) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_THEN_ELSE_3) : (&&PROCEED_NEXT_2));
PROCEED_NEXT_2:;
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(4);
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_2) : (&&PROCEED_THEN_ELSE_3));
goto PROCEED_THEN_ELSE_3;
PROCEED_THEN_ELSE_3:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_THEN_ELSE_4) : (&&PROCEED_NEXT_3));
PROCEED_NEXT_3:;
 ::TopLevel::i+=(0);
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_3) : (&&PROCEED_THEN_ELSE_4));
goto PROCEED_THEN_ELSE_4;
PROCEED_THEN_ELSE_4:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&PROCEED_THEN_ELSE_5) : (&&PROCEED_NEXT_4));
PROCEED_NEXT_4:;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
 ::TopLevel::i++;
goto *(( ::TopLevel::i<(4)) ? (&&PROCEED_NEXT_4) : (&&PROCEED_THEN_ELSE_5));
goto PROCEED_THEN_ELSE_5;
PROCEED_THEN_ELSE_5:;
cease(  ::TopLevel::x );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}