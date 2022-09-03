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
void (f)();
/*temp*/ int f_i;
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
cease( ({ /*temp*/ int temp_i; temp_i=(1); ({ {
{
 ::TopLevel::f_i=temp_i;
{
 ::TopLevel::f_link=(&&LINK);
 ::TopLevel::f();
}
}
LINK:;
}
 ::TopLevel::f_return; }); }) );
return ;
}

void (TopLevel::f)()
{
/*temp*/ void *temp_link;
auto void *link =  ::TopLevel::f_link;
auto int i =  ::TopLevel::f_i;
{
 ::TopLevel::f_return=i;
{
temp_link=link;
return ;
}
}
}
