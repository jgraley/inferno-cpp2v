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
private:
int i;
public:
int x;
private:
void *link;
public:
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
 ::TopLevel::f_link=(&&LINK);
{
 ::TopLevel::f_i=temp_i;
goto ENTER_f;
}
}
LINK:;
}
 ::TopLevel::f_return; }); }) );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::link= ::TopLevel::f_link;
{
 ::TopLevel::f_return= ::TopLevel::i;
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}