#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(U);
}
void (HelperU)();
void U();
/*temp*/ void *HelperU_link1;
/*temp*/ void *HelperU_link;
};
TopLevel top_level("top_level");

void (TopLevel::HelperU)()
{
/*temp*/ void *temp_link;
auto void *link =  ::TopLevel::HelperU_link;
{
temp_link=link;
return ;
}
}

void TopLevel::U()
{
{
{
 ::TopLevel::HelperU_link=(&&LINK);
 ::TopLevel::HelperU();
}
LINK:;
}
return ;
}
