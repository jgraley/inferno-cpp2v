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

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(5)) )
goto THEN_1;
{
NEXT:;
{
{
 ::gvar+= ::i;
{
if( !(( ::i%(2))==(0)) )
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
CONTINUE_1:;
if(  ::i<(5) )
goto NEXT;
}
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
}
cease(  ::gvar );
return ;
}
