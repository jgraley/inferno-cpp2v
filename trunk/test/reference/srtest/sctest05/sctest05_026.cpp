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
int gvar;
TopLevel top_level("top_level");
int i;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(5)) )
goto THEN1;
{
NEXT:;
{
{
 ::gvar+= ::i;
{
if( !((0)==( ::i%(2))) )
goto THEN;
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
