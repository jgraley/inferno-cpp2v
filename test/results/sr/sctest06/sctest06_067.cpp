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
int gvar;
int i;
int j;

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
state=((!( ::j<(3))) ? (&&PROCEED_THEN_ELSE_1) : (&&PROCEED_NEXT_1));
goto *(state);
}
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
{
state=(&&YIELD);
goto *(state);
}
YIELD:;
 ::gvar++;
 ::j++;
{
state=(( ::j<(3)) ? (&&PROCEED_NEXT_1) : (&&PROCEED_THEN_ELSE_1));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::gvar*=(2);
 ::i++;
{
state=(( ::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}