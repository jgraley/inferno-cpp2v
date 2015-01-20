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
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
/*temp*/ int f_return;
private:
void *link;
public:
float fi;
int x;
int i;
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
goto *((!( ::TopLevel::i<(4))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
goto BREAK;
CONTINUE:;
 ::TopLevel::i++;
CONTINUE_1:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT) : (&&PROCEED_1));
PROCEED_1:;
goto ELSE;
THEN:;
ELSE:;
BREAK:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_1) : (&&PROCEED_2));
PROCEED_2:;
NEXT_1:;
goto CONTINUE_2;
CONTINUE_2:;
 ::TopLevel::i++;
CONTINUE_3:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT_1) : (&&PROCEED_3));
PROCEED_3:;
goto ELSE_1;
THEN_1:;
ELSE_1:;
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_2) : (&&PROCEED_4));
PROCEED_4:;
NEXT_2:;
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE_4:;
 ::TopLevel::i++;
CONTINUE_5:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT_2) : (&&PROCEED_5));
PROCEED_5:;
goto ELSE_2;
THEN_2:;
ELSE_2:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i< ::TopLevel::x)) ? (&&THEN_3) : (&&PROCEED_6));
PROCEED_6:;
NEXT_3:;
CONTINUE_6:;
 ::TopLevel::i++;
CONTINUE_7:;
goto *(( ::TopLevel::i< ::TopLevel::x) ? (&&NEXT_3) : (&&PROCEED_7));
PROCEED_7:;
goto ELSE_3;
THEN_3:;
ELSE_3:;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=( ::TopLevel::i+(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_4) : (&&PROCEED_8));
PROCEED_8:;
NEXT_4:;
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE_8:;
 ::TopLevel::i=(4);
CONTINUE_9:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT_4) : (&&PROCEED_9));
PROCEED_9:;
goto ELSE_4;
THEN_4:;
ELSE_4:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_5) : (&&PROCEED_10));
PROCEED_10:;
NEXT_5:;
 ::TopLevel::i+=(0);
CONTINUE_10:;
 ::TopLevel::i++;
CONTINUE_11:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT_5) : (&&PROCEED_11));
PROCEED_11:;
goto ELSE_5;
THEN_5:;
ELSE_5:;
 ::TopLevel::i=(0);
goto *((!( ::TopLevel::i<(4))) ? (&&THEN_6) : (&&PROCEED_12));
PROCEED_12:;
NEXT_6:;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
 ::TopLevel::x+=result;
CONTINUE_12:;
 ::TopLevel::i++;
CONTINUE_13:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT_6) : (&&PROCEED_13));
PROCEED_13:;
goto ELSE_6;
THEN_6:;
ELSE_6:;
cease(  ::TopLevel::x );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
