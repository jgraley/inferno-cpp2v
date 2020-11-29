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
float fi;
int i;
private:
void *link;
public:
int x;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
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
if(  ::TopLevel::i<(4) )
{
NEXT:;
{
goto BREAK;
CONTINUE:;
 ::TopLevel::i++;
}
CONTINUE1:;
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
NEXT1:;
{
goto CONTINUE2;
CONTINUE2:;
 ::TopLevel::i++;
}
CONTINUE3:;
if(  ::TopLevel::i<(4) )
goto NEXT1;
}
}
{
;
if(  ::TopLevel::i<(4) )
{
NEXT2:;
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE4:;
 ::TopLevel::i++;
}
CONTINUE5:;
if(  ::TopLevel::i<(4) )
goto NEXT2;
}
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i< ::TopLevel::x )
{
NEXT3:;
{
{
}
CONTINUE6:;
 ::TopLevel::i++;
}
CONTINUE7:;
if(  ::TopLevel::i< ::TopLevel::x )
goto NEXT3;
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
NEXT4:;
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE8:;
 ::TopLevel::i=(4);
}
CONTINUE9:;
if(  ::TopLevel::i<(4) )
goto NEXT4;
}
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
{
NEXT5:;
{
 ::TopLevel::i+=(0);
CONTINUE10:;
 ::TopLevel::i++;
}
CONTINUE11:;
if(  ::TopLevel::i<(4) )
goto NEXT5;
}
}
{
 ::TopLevel::i=(0);
if(  ::TopLevel::i<(4) )
{
NEXT6:;
{
 ::TopLevel::x+=({ {
{
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
CONTINUE12:;
 ::TopLevel::i++;
}
CONTINUE13:;
if(  ::TopLevel::i<(4) )
goto NEXT6;
}
}
cease(  ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link1;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
