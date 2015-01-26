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
/*temp*/ unsigned int HelperU_link;
/*temp*/ unsigned int HelperU_link1;
private:
unsigned int link;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&, && };
/*temp*/ unsigned int temp_link;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::HelperU_link1= ::TopLevel::U_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::U_STATE_ENTER_HelperU;
continue;
}
if(  ::TopLevel::U_STATE_LINK==state )
{
return ;
state= ::TopLevel::U_STATE_ENTER_HelperU;
}
if( state== ::TopLevel::U_STATE_ENTER_HelperU )
{
 ::TopLevel::link= ::TopLevel::HelperU_link1;
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
