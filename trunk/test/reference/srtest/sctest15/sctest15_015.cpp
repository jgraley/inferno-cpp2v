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
/*temp*/ void *HelperU_link;
/*temp*/ void *HelperU_link1;
void (HelperU)();
private:
void *link;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
{
{
 ::TopLevel::HelperU_link1=(&&LINK);
 ::TopLevel::HelperU();
}
LINK:;
}
return ;
}

void (TopLevel::HelperU)()
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::HelperU_link1;
{
temp_link= ::TopLevel::link;
return ;
}
}