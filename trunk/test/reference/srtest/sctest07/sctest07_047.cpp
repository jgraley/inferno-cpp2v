#include "isystemc.h"

class TopLevel;
int gvar;
int i;
int j;
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
wait(SC_ZERO_TIME);
goto *((!( ::i<(4))) ? (&&PROCEED_THEN_ELSE1) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar+= ::i;
 ::j=(0);
goto *((!( ::j<(3))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT1));
PROCEED_NEXT1:;
{
wait(SC_ZERO_TIME);
goto YIELD;
YIELD:;
}
 ::gvar++;
 ::j++;
goto *(( ::j<(3)) ? (&&PROCEED_NEXT1) : (&&PROCEED_THEN_ELSE));
PROCEED_THEN_ELSE:;
 ::gvar*=(2);
{
wait(SC_ZERO_TIME);
goto YIELD1;
YIELD1:;
}
 ::i++;
goto *(( ::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE1));
PROCEED_THEN_ELSE1:;
cease(  ::gvar );
return ;
}
