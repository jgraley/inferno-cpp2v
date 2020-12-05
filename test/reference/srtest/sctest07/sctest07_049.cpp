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
int j;
TopLevel top_level("top_level");

void TopLevel::T()
{
auto void *state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(4))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
goto *(state);
}
PROCEED_NEXT:;
 ::gvar+= ::i;
 ::j=(0);
{
state=((!( ::j<(3))) ? (&&PROCEED_THEN_ELSE1) : (&&PROCEED_NEXT1));
goto *(state);
}
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
{
state=(&&YIELD);
goto *(state);
}
YIELD:;
 ::gvar++;
 ::j++;
{
state=(( ::j<(3)) ? (&&PROCEED_NEXT1) : (&&PROCEED_THEN_ELSE1));
goto *(state);
}
PROCEED_THEN_ELSE1:;
 ::gvar*=(2);
wait(SC_ZERO_TIME);
{
state=(&&YIELD1);
goto *(state);
}
YIELD1:;
 ::i++;
{
state=(( ::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}
