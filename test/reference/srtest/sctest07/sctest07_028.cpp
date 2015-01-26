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
int i;
TopLevel top_level("top_level");
int j;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
goto *((!( ::i<(4))) ? (&&THEN1) : (&&PROCEED));
PROCEED:;
{
NEXT:;
{
{
 ::gvar+= ::i;
{
 ::j=(0);
{
goto *((!( ::j<(3))) ? (&&THEN) : (&&PROCEED1));
PROCEED1:;
{
NEXT1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
CONTINUE:;
 ::j++;
}
CONTINUE1:;
goto *(( ::j<(3)) ? (&&NEXT1) : (&&PROCEED2));
PROCEED2:;
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
CONTINUE2:;
 ::i++;
}
CONTINUE3:;
goto *(( ::i<(4)) ? (&&NEXT) : (&&PROCEED3));
PROCEED3:;
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
