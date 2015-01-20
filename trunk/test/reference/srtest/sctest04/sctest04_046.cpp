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
goto *((!( ::i<(5))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar+= ::i;
{
wait(SC_ZERO_TIME);
goto YIELD;
YIELD:;
}
 ::gvar*=(2);
 ::i++;
goto *(( ::i<(5)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}
