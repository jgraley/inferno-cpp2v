#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(U);
}
/*temp*/ void *HelperU_link;
void U();
void (HelperU)();
/*temp*/ void *HelperU_link_1;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
{
{
 ::TopLevel::HelperU_link_1=(&&LINK);
 ::TopLevel::HelperU();
}
LINK:;
}
return ;
}

void (TopLevel::HelperU)()
{
auto void *link;
/*temp*/ void *temp_link;
link= ::TopLevel::HelperU_link_1;
{
temp_link=link;
return ;
}
}
