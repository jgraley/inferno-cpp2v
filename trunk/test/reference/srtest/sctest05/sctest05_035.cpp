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
 ::i=(0);
goto *((!( ::i<(5))) ? (&&THEN_ELSE1) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar+= ::i;
goto *((!(( ::i%(2))==(0))) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
wait(SC_ZERO_TIME);
 ::gvar^=(1);
goto THEN_ELSE;
THEN_ELSE:;
 ::gvar*=(2);
 ::i++;
goto *(( ::i<(5)) ? (&&PROCEED_NEXT) : (&&PROCEED1));
PROCEED1:;
goto THEN_ELSE1;
THEN_ELSE1:;
cease(  ::gvar );
return ;
}