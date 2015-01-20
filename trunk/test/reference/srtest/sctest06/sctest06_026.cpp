#include "isystemc.h"

class TopLevel;
int j;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
};
int i;
int gvar;
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(4)) )
goto THEN_1;
{
NEXT:;
{
{
 ::gvar+= ::i;
{
 ::j=(0);
{
if( !( ::j<(3)) )
goto THEN;
{
NEXT_1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
CONTINUE:;
 ::j++;
}
CONTINUE_1:;
if(  ::j<(3) )
goto NEXT_1;
}
goto ELSE;
THEN:;
;
ELSE:;
}
}
 ::gvar*=(2);
}
CONTINUE_2:;
 ::i++;
}
CONTINUE_3:;
if(  ::i<(4) )
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
