#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_ENTER_f = 1,
T_STATE_LINK = 0,
};
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
static const void *(lmap[]) = { &&LINK, &&ENTER_f };
auto void *state;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int temp_i;
temp_i=(1);
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK]);
 ::TopLevel::f_i=temp_i;
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK:;
result= ::TopLevel::f_return;
result_1=result;
cease( result_1 );
return ;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
ENTER_f:;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return= ::TopLevel::i;
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
