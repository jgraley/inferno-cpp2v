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
int j;
int gvar;
int i;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(4)) )
goto THEN_1;
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
goto THEN;
goto PROCEED_1;
PROCEED_1:;
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
goto PROCEED_2;
PROCEED_2:;
}
goto ELSE;
THEN:;
;
ELSE:;
}
}
 ::gvar*=(2);
wait(SC_ZERO_TIME);
}
CONTINUE_2:;
 ::i++;
}
CONTINUE_3:;
if(  ::i<(4) )
goto NEXT;
goto PROCEED_3;
PROCEED_3:;
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
