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
 ::gvar=(1);
{
 ::i=(0);
{
goto *((!( ::i<(4))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
{
NEXT:;
{
{
 ::gvar+= ::i;
{
 ::j=(0);
{
goto *((!( ::j<(3))) ? (&&THEN_1) : (&&PROCEED_1));
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
THEN_1:;
;
ELSE:;
}
}
 ::gvar*=(2);
}
CONTINUE_2:;
 ::i++;
}
CONTINUE_3:;
goto *(( ::i<(4)) ? (&&NEXT) : (&&PROCEED_3));
PROCEED_3:;
}
goto ELSE_1;
THEN:;
;
ELSE_1:;
}
}
cease(  ::gvar );
return ;
}
