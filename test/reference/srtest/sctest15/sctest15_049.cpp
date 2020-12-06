#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(U);
}
void U();
private:
void *link;
public:
/*temp*/ void *HelperU_link;
/*temp*/ void *HelperU_link1;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
/*temp*/ void *temp_link;
auto void *state;
 ::TopLevel::HelperU_link=(&&LINK);
wait(SC_ZERO_TIME);
{
state=(&&ENTER_HelperU);
goto *(state);
}
LINK:;
return ;
{
state=(&&ENTER_HelperU);
goto *(state);
}
ENTER_HelperU:;
 ::TopLevel::link= ::TopLevel::HelperU_link;
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
