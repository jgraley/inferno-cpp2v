#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
private:
int i;
char k;
public:
/*temp*/ char f_k;
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK2 = 2U,
};
/*temp*/ int f_return;
/*temp*/ short f_j;
private:
void *link;
short j;
public:
/*temp*/ int f_i;
int x;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
private:
int t;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_k;
/*temp*/ int result;
/*temp*/ int temp_k1;
/*temp*/ int result1;
static const void *(lmap[]) = { &&LINK, &&LINK1, &&LINK2, &&ENTER_f };
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int result4;
auto void *state;
/*temp*/ int temp_j;
/*temp*/ int result5;
/*temp*/ int temp_j1;
/*temp*/ int temp_k2;
/*temp*/ int temp_i;
/*temp*/ int result6;
/*temp*/ int temp_i1;
/*temp*/ int temp_j2;
/*temp*/ int temp_i2;
/*temp*/ int result7;
/*temp*/ void *temp_link;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int result11;
 ::TopLevel::x=(0);
temp_k2=(0);
temp_i=(0);
temp_j1=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK1]);
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK:;
result1= ::TopLevel::f_return;
result5=result1;
result8=result5;
result6=result8;
temp_i1=result6;
temp_k1=(8);
temp_j=(6);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK1:;
result7= ::TopLevel::f_return;
result2=result7;
result=result2;
result10=result;
 ::TopLevel::x=result10;
temp_j2=(2);
temp_k=(3);
temp_i2=(1);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link=(lmap[ ::TopLevel::T_STATE_LINK2]);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK2:;
result4= ::TopLevel::f_return;
result11=result4;
result9=result11;
result3=result9;
cease(  ::TopLevel::x+((2)*result3) );
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
