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
unsigned int link;
public:
enum UStates
{
U_STATE_LINK = 0U,
U_STATE_ENTER_HelperU = 1U,
};
/*temp*/ unsigned int HelperU_link;
/*temp*/ unsigned int HelperU_link1;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&LINK, &&ENTER_HelperU };
auto unsigned int state;
 ::TopLevel::HelperU_link1= ::TopLevel::U_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::U_STATE_ENTER_HelperU;
LINK:;
if( state== ::TopLevel::U_STATE_LINK )
{
return ;
state= ::TopLevel::U_STATE_ENTER_HelperU;
}
ENTER_HelperU:;
if(  ::TopLevel::U_STATE_ENTER_HelperU==state )
{
 ::TopLevel::link= ::TopLevel::HelperU_link1;
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}