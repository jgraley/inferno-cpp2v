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
 ::i=(0);
goto *((!( ::i<(5))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
 ::gvar*=(2);
CONTINUE:;
 ::i++;
CONTINUE1:;
goto *(( ::i<(5)) ? (&&NEXT) : (&&PROCEED1));
PROCEED1:;
goto ELSE;
THEN:;
;
ELSE:;
cease(  ::gvar );
return ;
}
