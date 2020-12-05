#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(U);
}
enum UStates
{
U_STATE_ENTER_HelperU = 1U,
U_STATE_LINK = 0U,
};
void U();
private:
void *link;
public:
/*temp*/ void *HelperU_link1;
/*temp*/ void *HelperU_link;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
/*temp*/ void *temp_link;
static const void *(lmap[]) = { &&LINK, &&ENTER_HelperU };
auto void *state;
 ::TopLevel::HelperU_link=(lmap[ ::TopLevel::U_STATE_LINK]);
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
 ::TopLevel::link= ::TopLevel::HelperU_link;
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
