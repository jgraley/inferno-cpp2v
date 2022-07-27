#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
int x;
int (f)(auto int i);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
cease( ({ /*temp*/ int temp_i; temp_i=(1);  ::TopLevel::f(temp_i); }) );
return ;
}

int (TopLevel::f)(int i)
{
return i;
}
