#include "isystemc.h"

class TopLevel;
int i;
int gvar;
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

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(5)) )
goto THEN_1;
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
goto PROCEED_1;
PROCEED_1:;
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
goto PROCEED_2;
PROCEED_2:;
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
