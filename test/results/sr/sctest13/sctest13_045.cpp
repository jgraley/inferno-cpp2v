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
{
{
 ::TopLevel::i=(0);
{
if( !( ::TopLevel::i<(4)) )
goto THEN;
goto PROCEED;
PROCEED:;
{
NEXT:;
{
goto BREAK;
CONTINUE:;
 ::TopLevel::i++;
}
CONTINUE_1:;
if(  ::TopLevel::i<(4) )
goto NEXT;
goto PROCEED_1;
PROCEED_1:;
}
goto ELSE;
THEN:;
;
ELSE:;
}
}
BREAK:;
}
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i++ )
switch( 0 )
{
case 0:;
break;
}
{
 ::TopLevel::i=(0);
{
if( !( ::TopLevel::i<(4)) )
goto THEN_1;
goto PROCEED_2;
PROCEED_2:;
{
NEXT_1:;
{
goto CONTINUE_2;
CONTINUE_2:;
 ::TopLevel::i++;
}
CONTINUE_3:;
if(  ::TopLevel::i<(4) )
goto NEXT_1;
goto PROCEED_3;
PROCEED_3:;
}
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
}
{
;
{
if( !( ::TopLevel::i<(4)) )
goto THEN_2;
goto PROCEED_4;
PROCEED_4:;
{
NEXT_2:;
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE_4:;
 ::TopLevel::i++;
}
CONTINUE_5:;
if(  ::TopLevel::i<(4) )
goto NEXT_2;
goto PROCEED_5;
PROCEED_5:;
}
goto ELSE_2;
THEN_2:;
;
ELSE_2:;
}
}
{
 ::TopLevel::i=(0);
{
if( !( ::TopLevel::i< ::TopLevel::x) )
goto THEN_3;
goto PROCEED_6;
PROCEED_6:;
{
NEXT_3:;
{
{
}
CONTINUE_6:;
 ::TopLevel::i++;
}
CONTINUE_7:;
if(  ::TopLevel::i< ::TopLevel::x )
goto NEXT_3;
goto PROCEED_7;
PROCEED_7:;
}
goto ELSE_3;
THEN_3:;
;
ELSE_3:;
}
}
;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=((1)+ ::TopLevel::i) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
{
 ::TopLevel::i=(0);
{
if( !( ::TopLevel::i<(4)) )
goto THEN_4;
goto PROCEED_8;
PROCEED_8:;
{
NEXT_4:;
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE_8:;
 ::TopLevel::i=(4);
}
CONTINUE_9:;
if(  ::TopLevel::i<(4) )
goto NEXT_4;
goto PROCEED_9;
PROCEED_9:;
}
goto ELSE_4;
THEN_4:;
;
ELSE_4:;
}
}
{
 ::TopLevel::i=(0);
{
if( !( ::TopLevel::i<(4)) )
goto THEN_5;
goto PROCEED_10;
PROCEED_10:;
{
NEXT_5:;
{
 ::TopLevel::i+=(0);
CONTINUE_10:;
 ::TopLevel::i++;
}
CONTINUE_11:;
if(  ::TopLevel::i<(4) )
goto NEXT_5;
goto PROCEED_11;
PROCEED_11:;
}
goto ELSE_5;
THEN_5:;
;
ELSE_5:;
}
}
{
 ::TopLevel::i=(0);
{
if( !( ::TopLevel::i<(4)) )
goto THEN_6;
goto PROCEED_12;
PROCEED_12:;
{
NEXT_6:;
{
 ::TopLevel::x+=({ {
{
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
CONTINUE_12:;
 ::TopLevel::i++;
}
CONTINUE_13:;
if(  ::TopLevel::i<(4) )
goto NEXT_6;
goto PROCEED_13;
PROCEED_13:;
}
goto ELSE_6;
THEN_6:;
;
ELSE_6:;
}
}
cease(  ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
