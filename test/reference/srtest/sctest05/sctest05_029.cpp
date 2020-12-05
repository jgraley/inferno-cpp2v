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
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
goto *((!( ::i<(5))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
{
NEXT:;
{
{
 ::gvar+= ::i;
{
goto *((!((0)==( ::i%(2)))) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
{
wait(SC_ZERO_TIME);
 ::gvar^=(1);
}
goto ELSE;
THEN1:;
;
ELSE:;
}
 ::gvar*=(2);
}
CONTINUE:;
 ::i++;
}
CONTINUE1:;
goto *(( ::i<(5)) ? (&&NEXT) : (&&PROCEED2));
PROCEED2:;
}
goto ELSE1;
THEN:;
;
ELSE1:;
}
}
cease(  ::gvar );
return ;
}
