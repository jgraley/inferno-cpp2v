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
void (HelperU)();
};
TopLevel top_level("top_level");

void TopLevel::U()
{
 ::TopLevel::HelperU();
}

void (TopLevel::HelperU)()
{
}
