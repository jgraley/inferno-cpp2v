#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(U);
}
enum UStates
{
U_STATE_ENTER_HelperU = 1U,
U_STATE_LINK = 0U,
};
/*temp*/ unsigned int temp_link;
void U();
/*temp*/ unsigned int HelperU_link;
private:
unsigned int state;
unsigned int link;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::HelperU_link= ::TopLevel::U_STATE_LINK;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::U_STATE_ENTER_HelperU;
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::U_STATE_LINK) )
{
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::U_STATE_ENTER_HelperU;
}
if( enabled&&( ::TopLevel::U_STATE_ENTER_HelperU== ::TopLevel::state) )
{
 ::TopLevel::link= ::TopLevel::HelperU_link;
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
