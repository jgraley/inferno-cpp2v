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
enum UStates
{
U_STATE_ENTER_HelperU = 1U,
U_STATE_LINK = 0U,
};
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
static const void *(lmap[]) = { &&LINK, &&ENTER_HelperU };
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
