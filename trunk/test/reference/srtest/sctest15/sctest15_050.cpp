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
void U();
/*temp*/ void *HelperU_link1;
enum UStates
{
U_STATE_LINK = 0U,
U_STATE_ENTER_HelperU = 1U,
};
private:
void *link;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
auto void *state;
static const void *(lmap[]) = { &&LINK, &&ENTER_HelperU };
/*temp*/ void *temp_link;
 ::TopLevel::HelperU_link1=(lmap[ ::TopLevel::U_STATE_LINK]);
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::U_STATE_ENTER_HelperU]);
goto *(state);
}
LINK:;
return ;
{
state=(lmap[ ::TopLevel::U_STATE_ENTER_HelperU]);
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