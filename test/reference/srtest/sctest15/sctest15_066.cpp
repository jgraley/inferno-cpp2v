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
U_STATE_LINK = 0U,
U_STATE_ENTER_HelperU = 1U,
};
/*temp*/ unsigned int HelperU_link;
/*temp*/ unsigned int HelperU_link1;
private:
unsigned int link;
public:
void U();
};
TopLevel top_level("top_level");

void TopLevel::U()
{
auto unsigned int state;
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&ENTER_HelperU_LINK, &&ENTER_HelperU_LINK };
ENTER_HelperU_LINK:;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::HelperU_link= ::TopLevel::U_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::U_STATE_ENTER_HelperU;
goto *(lmap[state]);
}
if(  ::TopLevel::U_STATE_LINK==state )
{
return ;
state= ::TopLevel::U_STATE_ENTER_HelperU;
}
if( state== ::TopLevel::U_STATE_ENTER_HelperU )
{
 ::TopLevel::link= ::TopLevel::HelperU_link;
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
