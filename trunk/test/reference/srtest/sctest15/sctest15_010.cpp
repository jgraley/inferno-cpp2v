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
void (HelperU)(auto void *link);
void U();
};
TopLevel top_level("top_level");

void (TopLevel::HelperU)(void *link)
{
/*temp*/ void *temp_link;
{
temp_link=link;
return ;
}
}

void TopLevel::U()
{
{
 ::TopLevel::HelperU(&&LINK);
LINK:;
}
return ;
}
