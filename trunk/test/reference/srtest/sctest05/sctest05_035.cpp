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
TopLevel top_level("top_level");
int i;
int gvar;

void TopLevel::T()
{
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(5))) ? (&&THEN_ELSE_1) : (&&PROCEED_NEXT));
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
goto *(( ::i<(5)) ? (&&PROCEED_NEXT) : (&&PROCEED_1));
PROCEED_1:;
goto THEN_ELSE_1;
THEN_ELSE_1:;
cease(  ::gvar );
return ;
}
