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
void (f)(auto int i, auto void *link);
/*temp*/ int f_return;
/*temp*/ void *f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
cease( ({ /*temp*/ int temp_i; temp_i=(1); ({ {
 ::TopLevel::f(temp_i, &&LINK);
LINK:;
}
 ::TopLevel::f_return; }); }) );
return ;
}

void (TopLevel::f)(int i, void *link)
{
/*temp*/ void *temp_link;
{
 ::TopLevel::f_return=i;
{
temp_link=link;
return ;
}
}
}
