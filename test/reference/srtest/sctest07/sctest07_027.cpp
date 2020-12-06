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
};
TopLevel top_level("top_level");
int gvar;
int i;
int j;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(4)) )
goto THEN;
goto PROCEED;
PROCEED:;
{
NEXT:;
{
{
 ::gvar+= ::i;
{
 ::j=(0);
{
if( !( ::j<(3)) )
goto THEN1;
goto PROCEED1;
PROCEED1:;
{
NEXT1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
CONTINUE:;
 ::j++;
}
CONTINUE1:;
if(  ::j<(3) )
goto NEXT1;
goto PROCEED2;
PROCEED2:;
}
goto ELSE;
THEN1:;
;
ELSE:;
}
}
 ::gvar*=(2);
wait(SC_ZERO_TIME);
}
CONTINUE2:;
 ::i++;
}
CONTINUE3:;
if(  ::i<(4) )
goto NEXT;
goto PROCEED3;
PROCEED3:;
}
goto ELSE1;
THEN:;
;
ELSE1:;
}
}
cease(  ::gvar );
return ;
}
