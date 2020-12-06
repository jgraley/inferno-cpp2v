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
/*temp*/ void *HelperU_link1;
};
TopLevel top_level("top_level");

void TopLevel::U()
{
/*temp*/ void *temp_link;
 ::TopLevel::HelperU_link=(&&LINK);
goto ENTER_HelperU;
LINK:;
return ;
goto ENTER_HelperU;
ENTER_HelperU:;
 ::TopLevel::link= ::TopLevel::HelperU_link;
temp_link= ::TopLevel::link;
goto *(temp_link);
}
