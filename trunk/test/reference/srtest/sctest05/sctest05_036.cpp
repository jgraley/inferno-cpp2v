#include "isystemc.h"

class TopLevel;
int gvar;
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
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(5))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
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
goto *(( ::i<(5)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto PROCEED_THEN_ELSE;
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}