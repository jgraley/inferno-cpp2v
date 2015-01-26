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
T_STATE_LINK = 0U,
T_STATE_LINK1 = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK2 = 1U,
};
/*temp*/ char f_k;
private:
void *link;
short j;
public:
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
int x;
void T();
private:
char k;
public:
/*temp*/ short f_j;
private:
int i;
int t;
public:
/*temp*/ int f_i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int result3;
static const void *(lmap[]) = { &&LINK, &&LINK1, &&LINK2, &&ENTER_f };
auto void *state;
/*temp*/ int result4;
/*temp*/ int temp_i;
/*temp*/ int temp_j;
/*temp*/ int temp_k;
/*temp*/ void *temp_link;
/*temp*/ int temp_j1;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int temp_k1;
/*temp*/ int result7;
/*temp*/ int temp_k2;
/*temp*/ int result8;
/*temp*/ int temp_i1;
/*temp*/ int result9;
/*temp*/ int temp_j2;
/*temp*/ int temp_i2;
/*temp*/ int result10;
/*temp*/ int result11;
 ::TopLevel::x=(0);
temp_k2=(8);
temp_j2=(6);
temp_j1=(0);
temp_i1=(0);
temp_k1=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK]);
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK:;
result2= ::TopLevel::f_return;
result=result2;
result4=result;
result6=result4;
temp_i=result6;
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK2]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK1:;
result10= ::TopLevel::f_return;
result1=result10;
result7=result1;
result11=result7;
 ::TopLevel::x=result11;
temp_i2=(1);
temp_k=(3);
temp_j=(2);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK1]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK2:;
result3= ::TopLevel::f_return;
result8=result3;
result9=result8;
result5=result9;
cease(  ::TopLevel::x+(result5*(2)) );
return ;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
