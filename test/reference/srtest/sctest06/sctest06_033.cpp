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
int i;
int j;
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(4))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
 ::gvar+= ::i;
 ::j=(0);
goto *((!( ::j<(3))) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
NEXT1:;
wait(SC_ZERO_TIME);
 ::gvar++;
CONTINUE:;
 ::j++;
CONTINUE1:;
goto *(( ::j<(3)) ? (&&NEXT1) : (&&PROCEED2));
PROCEED2:;
goto ELSE;
THEN1:;
ELSE:;
 ::gvar*=(2);
CONTINUE2:;
 ::i++;
CONTINUE3:;
goto *(( ::i<(4)) ? (&&NEXT) : (&&PROCEED3));
PROCEED3:;
goto ELSE1;
THEN:;
ELSE1:;
cease(  ::gvar );
return ;
}
