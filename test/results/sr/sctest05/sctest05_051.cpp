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

void TopLevel::T()
{
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(5))) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
 ::gvar+= ::i;
goto *((!((0)==( ::i%(2)))) ? (&&THEN_1) : (&&PROCEED_1));
PROCEED_1:;
wait(SC_ZERO_TIME);
 ::gvar^=(1);
goto ELSE;
THEN_1:;
ELSE:;
 ::gvar*=(2);
CONTINUE:;
 ::i++;
CONTINUE_1:;
goto *(( ::i<(5)) ? (&&NEXT) : (&&PROCEED_2));
PROCEED_2:;
goto ELSE_1;
THEN:;
ELSE_1:;
cease(  ::gvar );
return ;
}