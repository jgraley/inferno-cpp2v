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
 ::i=(0);
goto *((!( ::i<(5))) ? (&&THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar+= ::i;
goto *((!((0)==( ::i%(2)))) ? (&&THEN_ELSE1) : (&&PROCEED));
PROCEED:;
wait(SC_ZERO_TIME);
 ::gvar^=(1);
goto THEN_ELSE1;
THEN_ELSE1:;
 ::gvar*=(2);
 ::i++;
goto *(( ::i<(5)) ? (&&PROCEED_NEXT) : (&&PROCEED1));
PROCEED1:;
goto THEN_ELSE;
THEN_ELSE:;
cease(  ::gvar );
return ;
}
