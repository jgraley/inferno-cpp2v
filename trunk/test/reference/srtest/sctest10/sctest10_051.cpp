#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
private:
char k;
unsigned int link;
int i;
public:
/*temp*/ int f_i;
enum TStates
{
T_STATE_LINK = 0U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK1 = 2U,
T_STATE_LINK2 = 1U,
};
private:
int t;
public:
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link1;
/*temp*/ char f_k;
void T();
/*temp*/ short f_j;
private:
short j;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
auto unsigned int state;
/*temp*/ int result1;
/*temp*/ int temp_k;
/*temp*/ unsigned int temp_link;
/*temp*/ int result2;
/*temp*/ int temp_i;
static const unsigned int (lmap[]) = { &&LINK, &&LINK1, &&LINK2, &&ENTER_f };
/*temp*/ int temp_i1;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_j;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int temp_k1;
/*temp*/ int temp_k2;
/*temp*/ int temp_i2;
/*temp*/ int temp_j1;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int result11;
/*temp*/ int temp_j2;
 ::TopLevel::x=(0);
temp_k2=(8);
temp_j=(6);
temp_j2=(0);
temp_i=(0);
temp_k=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
wait(SC_ZERO_TIME);
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK:;
result= ::TopLevel::f_return;
result4=result;
result8=result4;
result10=result8;
temp_i2=result10;
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK1:;
result5= ::TopLevel::f_return;
result9=result5;
result11=result9;
result6=result11;
 ::TopLevel::x=result6;
temp_i1=(1);
temp_k1=(3);
temp_j1=(2);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK2:;
result3= ::TopLevel::f_return;
result1=result3;
result7=result1;
result2=result7;
cease(  ::TopLevel::x+(result2*(2)) );
return ;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
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
goto *(lmap[state]);
}
}
