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
int j;
TopLevel top_level("top_level");
int i;

void TopLevel::T()
{
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(4))) ? (&&THEN_ELSE_1) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar+= ::i;
 ::j=(0);
goto *((!( ::j<(3))) ? (&&THEN_ELSE) : (&&PROCEED_NEXT_1));
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
 ::gvar++;
 ::j++;
goto *(( ::j<(3)) ? (&&PROCEED_NEXT_1) : (&&PROCEED));
PROCEED:;
goto THEN_ELSE;
THEN_ELSE:;
 ::gvar*=(2);
 ::i++;
goto *(( ::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_1));
PROCEED_1:;
goto THEN_ELSE_1;
THEN_ELSE_1:;
cease(  ::gvar );
return ;
}
