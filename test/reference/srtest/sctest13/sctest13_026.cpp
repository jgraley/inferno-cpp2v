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
int x;
/*temp*/ void *f_link1;
private:
void *link;
public:
float fi;
/*temp*/ int f_return;
int i;
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
{
if( !( ::TopLevel::i<(4)) )
goto THEN;
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
goto THEN1;
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
goto ELSE1;
THEN1:;
;
ELSE1:;
}
}
{
;
{
if( !( ::TopLevel::i<(4)) )
goto THEN2;
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
goto ELSE2;
THEN2:;
;
ELSE2:;
}
}
{
 ::TopLevel::i=(0);
{
if( !( ::TopLevel::i< ::TopLevel::x) )
goto THEN3;
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
goto ELSE3;
THEN3:;
;
ELSE3:;
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
{
if( !( ::TopLevel::i<(4)) )
goto THEN4;
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
goto ELSE4;
THEN4:;
;
ELSE4:;
}
}
{
 ::TopLevel::i=(0);
{
if( !( ::TopLevel::i<(4)) )
goto THEN5;
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
goto ELSE5;
THEN5:;
;
ELSE5:;
}
}
{
 ::TopLevel::i=(0);
{
if( !( ::TopLevel::i<(4)) )
goto THEN6;
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
goto ELSE6;
THEN6:;
;
ELSE6:;
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
