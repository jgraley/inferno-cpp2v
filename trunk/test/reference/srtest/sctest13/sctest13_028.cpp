#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
float fi;
/*temp*/ void *f_link;
int x;
void T();
private:
void *link;
public:
int i;
/*temp*/ int f_return;
/*temp*/ void *f_link1;
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
goto *((!( ::TopLevel::i<(4))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
{
NEXT:;
{
goto BREAK;
CONTINUE:;
 ::TopLevel::i++;
}
CONTINUE1:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT) : (&&PROCEED1));
PROCEED1:;
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
goto *((!( ::TopLevel::i<(4))) ? (&&THEN1) : (&&PROCEED2));
PROCEED2:;
{
NEXT1:;
{
goto CONTINUE2;
CONTINUE2:;
 ::TopLevel::i++;
}
CONTINUE3:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT1) : (&&PROCEED3));
PROCEED3:;
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
goto *((!( ::TopLevel::i<(4))) ? (&&THEN2) : (&&PROCEED4));
PROCEED4:;
{
NEXT2:;
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE4:;
 ::TopLevel::i++;
}
CONTINUE5:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT2) : (&&PROCEED5));
PROCEED5:;
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
goto *((!( ::TopLevel::i< ::TopLevel::x)) ? (&&THEN3) : (&&PROCEED6));
PROCEED6:;
{
NEXT3:;
{
{
}
CONTINUE6:;
 ::TopLevel::i++;
}
CONTINUE7:;
goto *(( ::TopLevel::i< ::TopLevel::x) ? (&&NEXT3) : (&&PROCEED7));
PROCEED7:;
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
goto *((!( ::TopLevel::i<(4))) ? (&&THEN4) : (&&PROCEED8));
PROCEED8:;
{
NEXT4:;
{
 ::TopLevel::x+= ::TopLevel::i;
CONTINUE8:;
 ::TopLevel::i=(4);
}
CONTINUE9:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT4) : (&&PROCEED9));
PROCEED9:;
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
goto *((!( ::TopLevel::i<(4))) ? (&&THEN5) : (&&PROCEED10));
PROCEED10:;
{
NEXT5:;
{
 ::TopLevel::i+=(0);
CONTINUE10:;
 ::TopLevel::i++;
}
CONTINUE11:;
goto *(( ::TopLevel::i<(4)) ? (&&NEXT5) : (&&PROCEED11));
PROCEED11:;
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
goto *((!( ::TopLevel::i<(4))) ? (&&THEN6) : (&&PROCEED12));
PROCEED12:;
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
goto *(( ::TopLevel::i<(4)) ? (&&NEXT6) : (&&PROCEED13));
PROCEED13:;
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
