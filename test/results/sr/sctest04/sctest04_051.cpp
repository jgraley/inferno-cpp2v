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
private:
int t;
};
TopLevel top_level("top_level");
int gvar;
int i;

void TopLevel::T()
{
 ::gvar=(1);
 ::TopLevel::t=(5);
 ::i=(0);
goto *((!( ::i< ::TopLevel::t)) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
 ::gvar*=(2);
CONTINUE:;
 ::i++;
CONTINUE_1:;
goto *(( ::i< ::TopLevel::t) ? (&&NEXT) : (&&PROCEED_1));
PROCEED_1:;
goto ELSE;
THEN:;
ELSE:;
cease(  ::gvar );
return ;
}