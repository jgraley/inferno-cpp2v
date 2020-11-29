#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(U);
}
/*temp*/ unsigned int temp_link;
enum UStates
{
U_STATE_LINK = 0U,
U_STATE_ENTER_HelperU = 1U,
};
void U();
private:
unsigned int link;
unsigned int state;
public:
/*temp*/ unsigned int HelperU_link;
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
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::U_STATE_LINK )
{
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::U_STATE_ENTER_HelperU;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::U_STATE_ENTER_HelperU )
{
 ::TopLevel::link= ::TopLevel::HelperU_link;
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
