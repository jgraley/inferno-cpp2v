#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(U);
}
/*temp*/ unsigned int HelperU_link;
private:
unsigned int link;
public:
void U();
/*temp*/ unsigned int HelperU_link1;
enum UStates
{
U_STATE_LINK = 0U,
U_STATE_ENTER_HelperU = 1U,
};
};
TopLevel top_level("top_level");

void TopLevel::U()
{
/*temp*/ unsigned int temp_link;
auto unsigned int state;
static const unsigned int (lmap[]) = { &&LINK, &&ENTER_HelperU };
 ::TopLevel::HelperU_link1= ::TopLevel::U_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::U_STATE_ENTER_HelperU;
LINK:;
if(  ::TopLevel::U_STATE_LINK==state )
{
return ;
state= ::TopLevel::U_STATE_ENTER_HelperU;
}
ENTER_HelperU:;
if( state== ::TopLevel::U_STATE_ENTER_HelperU )
{
 ::TopLevel::link= ::TopLevel::HelperU_link1;
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
