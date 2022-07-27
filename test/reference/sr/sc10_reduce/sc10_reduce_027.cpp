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
void (f)(auto int i);
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
cease( ({ /*temp*/ int temp_i; temp_i=(1); ({  ::TopLevel::f(temp_i);  ::TopLevel::f_return; }); }) );
return ;
}

void (TopLevel::f)(int i)
{
{
 ::TopLevel::f_return=i;
return ;
}
}
