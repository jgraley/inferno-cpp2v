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
void SC_ZERO_TIME;
void (cease)(void p1);
void (exit)(void p1_1);
void (next_trigger)(void p1_2);
void (wait)(void p1_3);

void TopLevel::U()
{
 ::TopLevel::HelperU();
}

void (TopLevel::HelperU)()
{
}

void (cease)(void p1);

void (exit)(void p1_1);

void (next_trigger)(void p1_2);

void (wait)(void p1_3);