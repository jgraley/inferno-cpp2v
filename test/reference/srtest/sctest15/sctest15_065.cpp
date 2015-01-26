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
/*temp*/ unsigned int HelperU_link1;
enum UStates
{
U_STATE_LINK = 0U,
U_STATE_ENTER_HelperU = 1U,
};
private:
unsigned int link;
public:
void U();
};
TopLevel top_level("top_level");

void TopLevel::U()
{
static const unsigned int (lmap[]) = { &&ENTER_HelperU_LINK, &&ENTER_HelperU_LINK };
auto unsigned int state;
/*temp*/ unsigned int temp_link;
 ::TopLevel::HelperU_link= ::TopLevel::U_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::U_STATE_ENTER_HelperU;
ENTER_HelperU_LINK:;
if( state== ::TopLevel::U_STATE_LINK )
{
return ;
state= ::TopLevel::U_STATE_ENTER_HelperU;
}
if(  ::TopLevel::U_STATE_ENTER_HelperU==state )
{
 ::TopLevel::link= ::TopLevel::HelperU_link;
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
