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
};
TopLevel top_level("top_level");

void (TopLevel::HelperU)()
{
return ;
}

void TopLevel::U()
{
 ::TopLevel::HelperU();
return ;
}
