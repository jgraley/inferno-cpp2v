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
/*temp*/ unsigned int HelperU_link;
private:
unsigned int link;
public:
void U();
};
TopLevel top_level("top_level");

void TopLevel::U()
{
/*temp*/ unsigned int temp_link;
auto unsigned int state;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::HelperU_link= ::TopLevel::U_STATE_LINK;
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
 ::TopLevel::link= ::TopLevel::HelperU_link;
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
