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
enum UStates
{
U_STATE_LINK = 0U,
U_STATE_ENTER_HelperU = 1U,
};
private:
unsigned int state;
public:
void U();
/*temp*/ unsigned int temp_link;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::HelperU_link= ::TopLevel::U_STATE_LINK;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::U_STATE_ENTER_HelperU;
continue;
}
if(  ::TopLevel::state== ::TopLevel::U_STATE_LINK )
{
return ;
 ::TopLevel::state= ::TopLevel::U_STATE_ENTER_HelperU;
}
if(  ::TopLevel::state== ::TopLevel::U_STATE_ENTER_HelperU )
{
 ::TopLevel::link= ::TopLevel::HelperU_link;
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}