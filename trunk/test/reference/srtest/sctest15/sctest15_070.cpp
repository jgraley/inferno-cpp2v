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
U_STATE_ENTER_HelperU = 1U,
U_STATE_LINK = 0U,
};
/*temp*/ unsigned int HelperU_link;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
auto unsigned int state;
/*temp*/ unsigned int temp_link;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::HelperU_link= ::TopLevel::U_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::U_STATE_ENTER_HelperU;
continue;
}
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
}
while( true );
}