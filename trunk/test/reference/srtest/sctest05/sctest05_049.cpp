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
auto void *state;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
{
state=((!( ::i<(5))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
goto *(state);
}
PROCEED_NEXT:;
 ::gvar+= ::i;
{
state=((!((0)==( ::i%(2)))) ? (&&THEN_ELSE) : (&&PROCEED));
goto *(state);
}
PROCEED:;
wait(SC_ZERO_TIME);
{
state=(&&YIELD);
goto *(state);
}
YIELD:;
 ::gvar^=(1);
{
state=(&&THEN_ELSE);
goto *(state);
}
THEN_ELSE:;
 ::gvar*=(2);
 ::i++;
{
state=(( ::i<(5)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
}
