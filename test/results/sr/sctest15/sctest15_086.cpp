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
U_STATE_ENTER_HelperU = 1,
U_STATE_LINK = 0,
};
void U();
private:
unsigned int link;
public:
/*temp*/ unsigned int HelperU_link;
/*temp*/ unsigned int HelperU_link_1;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&ENTER_HelperU_LINK, &&ENTER_HelperU_LINK };
auto unsigned int state;
do
{
if( (sc_delta_count())==(0) )
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
if(  ::TopLevel::U_STATE_ENTER_HelperU==state )
{
 ::TopLevel::link= ::TopLevel::HelperU_link;
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}