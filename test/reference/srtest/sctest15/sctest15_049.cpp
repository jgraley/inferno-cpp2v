#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(U);
}
/*temp*/ void *HelperU_link;
/*temp*/ void *HelperU_link1;
private:
void *link;
public:
void U();
};
TopLevel top_level("top_level");

void TopLevel::U()
{
auto void *state;
/*temp*/ void *temp_link;
 ::TopLevel::HelperU_link1=(&&LINK);
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
 ::TopLevel::link= ::TopLevel::HelperU_link1;
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
