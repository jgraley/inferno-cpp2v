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
/*temp*/ void *HelperU_link1;
private:
void *link;
public:
void U();
};
TopLevel top_level("top_level");

void TopLevel::U()
{
{
{
 ::TopLevel::HelperU_link1=(&&LINK);
goto ENTER_HelperU;
}
LINK:;
}
return ;
ENTER_HelperU:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::HelperU_link1;
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
