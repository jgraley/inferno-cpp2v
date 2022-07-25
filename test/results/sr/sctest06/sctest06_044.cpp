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
{
NEXT:;
{
{
 ::gvar+= ::i;
{
 ::j=(0);
{
if( !( ::j<(3)) )
goto THEN_1;
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
THEN_1:;
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
THEN:;
;
ELSE_1:;
}
}
cease(  ::gvar );
return ;
}
