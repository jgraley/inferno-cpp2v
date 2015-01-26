#include "isystemc.h"

class TopLevel;
int i;
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
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(5))) ? (&&THEN1) : (&&PROCEED));
PROCEED:;
NEXT:;
 ::gvar+= ::i;
goto *((!((0)==( ::i%(2)))) ? (&&THEN) : (&&PROCEED1));
PROCEED1:;
wait(SC_ZERO_TIME);
 ::gvar^=(1);
goto ELSE;
THEN:;
ELSE:;
 ::gvar*=(2);
 ::i++;
goto *(( ::i<(5)) ? (&&NEXT) : (&&PROCEED2));
PROCEED2:;
goto ELSE1;
THEN1:;
ELSE1:;
cease(  ::gvar );
return ;
}
