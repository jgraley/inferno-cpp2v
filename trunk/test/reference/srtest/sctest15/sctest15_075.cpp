#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(U);
}
private:
unsigned int state;
public:
/*temp*/ unsigned int temp_link;
void U();
private:
unsigned int link;
public:
/*temp*/ unsigned int HelperU_link;
enum UStates
{
U_STATE_ENTER_HelperU = 1U,
U_STATE_LINK = 0U,
};
};
TopLevel top_level("top_level");

void TopLevel::U()
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::HelperU_link= ::TopLevel::U_STATE_LINK;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::U_STATE_ENTER_HelperU;
return ;
}
if(  ::TopLevel::state== ::TopLevel::U_STATE_LINK )
{
return ;
 ::TopLevel::state= ::TopLevel::U_STATE_ENTER_HelperU;
}
if(  ::TopLevel::U_STATE_ENTER_HelperU== ::TopLevel::state )
{
 ::TopLevel::link= ::TopLevel::HelperU_link;
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
next_trigger(SC_ZERO_TIME);
}
