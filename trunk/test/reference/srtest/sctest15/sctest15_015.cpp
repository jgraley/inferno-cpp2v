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
/*temp*/ void *HelperU_link;
private:
void *link;
public:
/*temp*/ void *HelperU_link1;
void U();
};
TopLevel top_level("top_level");

void (TopLevel::HelperU)()
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::HelperU_link;
{
temp_link= ::TopLevel::link;
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
