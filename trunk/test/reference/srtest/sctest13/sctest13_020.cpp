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
/*temp*/ void *f_link;
float fi;
void T();
private:
void *link;
public:
int i;
/*temp*/ int f_return;
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
if(  ::TopLevel::i<(4) )
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
if(  ::TopLevel::i<(4) )
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
}
}
{
;
if(  ::TopLevel::i<(4) )
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
}
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i< ::TopLevel::x )
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
}
}
;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i=( ::TopLevel::i+(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(0);  ::TopLevel::i<(4);  ::TopLevel::i+=(1) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i=( ::TopLevel::i-(1)) )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-=(1) )
 ::TopLevel::x+= ::TopLevel::i;
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
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
}
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
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
}
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
{
NEXT_6:;
{
 ::TopLevel::x+=({ {
{
 ::TopLevel::f_link_1=(&&LINK);
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
}
}
cease(  ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link_1;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
