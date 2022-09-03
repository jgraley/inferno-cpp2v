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
/*temp*/ void *temp_link;
auto void *state;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int temp_i;
temp_i=(1);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link=(&&LINK);
wait(SC_ZERO_TIME);
{
state=(&&ENTER_f);
goto *(state);
}
LINK:;
result= ::TopLevel::f_return;
result_1=result;
cease( result_1 );
return ;
{
state=(&&ENTER_f);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return= ::TopLevel::i;
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
