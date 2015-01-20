#include "isystemc.h"

class TopLevel;
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
int i;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(5)) )
goto THEN;
goto PROCEED;
PROCEED:;
{
NEXT:;
{
{
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
 ::gvar*=(2);
}
CONTINUE:;
 ::i++;
}
CONTINUE_1:;
if(  ::i<(5) )
goto NEXT;
goto PROCEED_1;
PROCEED_1:;
}
goto ELSE;
THEN:;
;
ELSE:;
}
}
cease(  ::gvar );
return ;
}
