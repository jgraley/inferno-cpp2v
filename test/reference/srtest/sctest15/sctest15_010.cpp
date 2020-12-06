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
void (HelperU)(auto void *link);
/*temp*/ void *HelperU_link;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
{
 ::TopLevel::HelperU(&&LINK);
LINK:;
}
return ;
}

void (TopLevel::HelperU)(void *link)
{
/*temp*/ void *temp_link;
{
temp_link=link;
return ;
}
}
