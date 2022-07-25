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
/*temp*/ void *HelperU_link;
/*temp*/ void *HelperU_link_1;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
{
{
 ::TopLevel::HelperU_link=(&&LINK);
goto ENTER_HelperU;
}
LINK:;
}
return ;
ENTER_HelperU:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::HelperU_link;
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
