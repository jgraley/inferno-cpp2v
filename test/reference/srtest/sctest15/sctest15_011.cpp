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
/*temp*/ void *HelperU_link_1;
void (HelperU)();
void U();
};
TopLevel top_level("top_level");

void (TopLevel::HelperU)()
{
/*temp*/ void *temp_link;
auto void *link =  ::TopLevel::HelperU_link_1;
{
temp_link=link;
return ;
}
}

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
