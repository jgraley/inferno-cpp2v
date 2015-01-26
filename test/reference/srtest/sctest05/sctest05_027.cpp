#include "isystemc.h"

class TopLevel;
int i;
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

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(5)) )
goto THEN1;
goto PROCEED;
PROCEED:;
{
NEXT:;
{
{
 ::gvar+= ::i;
{
if( !((0)==( ::i%(2))) )
goto THEN;
goto PROCEED1;
PROCEED1:;
{
wait(SC_ZERO_TIME);
 ::gvar^=(1);
}
goto ELSE;
THEN:;
;
ELSE:;
}
 ::gvar*=(2);
}
CONTINUE:;
 ::i++;
}
CONTINUE1:;
if(  ::i<(5) )
goto NEXT;
goto PROCEED2;
PROCEED2:;
}
goto ELSE1;
THEN1:;
;
ELSE1:;
}
}
cease(  ::gvar );
return ;
}
