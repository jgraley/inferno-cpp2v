#include "isystemc.h"

class TopLevel;
int i;
int j;
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

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
goto *((!( ::i<(4))) ? (&&THEN_1) : (&&PROCEED));
PROCEED:;
{
NEXT:;
{
{
 ::gvar+= ::i;
{
 ::j=(0);
{
goto *((!( ::j<(3))) ? (&&THEN) : (&&PROCEED_1));
PROCEED_1:;
{
NEXT_1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
CONTINUE:;
 ::j++;
}
CONTINUE_1:;
goto *(( ::j<(3)) ? (&&NEXT_1) : (&&PROCEED_2));
PROCEED_2:;
}
goto ELSE;
THEN:;
;
ELSE:;
}
}
 ::gvar*=(2);
wait(SC_ZERO_TIME);
}
CONTINUE_2:;
 ::i++;
}
CONTINUE_3:;
goto *(( ::i<(4)) ? (&&NEXT) : (&&PROCEED_3));
PROCEED_3:;
}
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
}
cease(  ::gvar );
return ;
}
