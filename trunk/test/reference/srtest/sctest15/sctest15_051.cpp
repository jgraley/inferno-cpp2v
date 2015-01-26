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
unsigned int link;
public:
/*temp*/ unsigned int HelperU_link;
/*temp*/ unsigned int HelperU_link1;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
static const unsigned int (lmap[]) = { &&LINK, &&ENTER_HelperU };
/*temp*/ unsigned int temp_link;
auto unsigned int state;
 ::TopLevel::HelperU_link= ::TopLevel::U_STATE_LINK;
wait(SC_ZERO_TIME);
{
state= ::TopLevel::U_STATE_ENTER_HelperU;
goto *(lmap[state]);
}
LINK:;
return ;
{
state= ::TopLevel::U_STATE_ENTER_HelperU;
goto *(lmap[state]);
}
ENTER_HelperU:;
 ::TopLevel::link= ::TopLevel::HelperU_link;
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(lmap[state]);
}
}
