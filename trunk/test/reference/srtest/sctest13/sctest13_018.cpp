#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *f_link;
private:
void *link;
public:
/*temp*/ int f_return;
float fi;
void T();
int x;
int i;
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
for(  ::TopLevel::i=(0);  ::TopLevel::i!=(4);  ::TopLevel::i++ )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
for(  ::TopLevel::i=(4);  ::TopLevel::i>=(0);  ::TopLevel::i-- )
 ::TopLevel::x+= ::TopLevel::i;
{
{
 ::TopLevel::i=(0);
while(  ::TopLevel::i<(4) )
{
goto BREAK;
CONTINUE:;
 ::TopLevel::i++;
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
while(  ::TopLevel::i<(4) )
{
goto CONTINUE_1;
CONTINUE_1:;
 ::TopLevel::i++;
}
}
{
;
while(  ::TopLevel::i<(4) )
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE_2:;
 ::TopLevel::i++;
}
}
{
 ::TopLevel::i=(0);
while(  ::TopLevel::i< ::TopLevel::x )
{
{
}
CONTINUE_3:;
 ::TopLevel::i++;
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
while(  ::TopLevel::i<(4) )
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE_4:;
 ::TopLevel::i=(4);
}
}
{
 ::TopLevel::i=(0);
while(  ::TopLevel::i<(4) )
{
 ::TopLevel::i+=(0);
CONTINUE_5:;
 ::TopLevel::i++;
}
}
{
 ::TopLevel::i=(0);
while(  ::TopLevel::i<(4) )
{
 ::TopLevel::x+=({ {
{
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
CONTINUE_6:;
 ::TopLevel::i++;
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
