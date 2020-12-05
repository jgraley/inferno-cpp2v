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
void U();
private:
unsigned int link;
public:
/*temp*/ unsigned int HelperU_link1;
/*temp*/ unsigned int HelperU_link;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&
#error identifier ENTER_HelperU_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_HelperU_LINK undeclared not supported in RenderIdentifier
 };
auto unsigned int state;
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
if( state== ::TopLevel::U_STATE_ENTER_HelperU )
{
 ::TopLevel::link= ::TopLevel::HelperU_link;
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
