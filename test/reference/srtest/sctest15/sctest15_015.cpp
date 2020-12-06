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
private:
void *link;
public:
void (HelperU)();
/*temp*/ void *HelperU_link;
/*temp*/ void *HelperU_link1;
};
TopLevel top_level("top_level");

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

void (TopLevel::HelperU)()
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::HelperU_link;
{
temp_link= ::TopLevel::link;
return ;
}
}
