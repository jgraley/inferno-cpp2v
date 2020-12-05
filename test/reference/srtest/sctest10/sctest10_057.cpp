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
T_STATE_ENTER_f = 3U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 1U,
T_STATE_LINK = 2U,
};
void T();
private:
int i;
short j;
char k;
unsigned int link;
int t;
public:
int x;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link1;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&LINK, &&LINK1, &&LINK2, &&ENTER_f };
auto unsigned int state;
/*temp*/ int result1;
/*temp*/ int result3;
/*temp*/ int result9;
/*temp*/ int result6;
/*temp*/ int result11;
/*temp*/ int result5;
/*temp*/ int result;
/*temp*/ int result8;
/*temp*/ int result10;
/*temp*/ int result4;
/*temp*/ int result7;
/*temp*/ int result2;
/*temp*/ int temp_i;
/*temp*/ int temp_i2;
/*temp*/ int temp_i1;
/*temp*/ int temp_j1;
/*temp*/ int temp_j;
/*temp*/ int temp_j2;
/*temp*/ int temp_k2;
/*temp*/ int temp_k;
/*temp*/ int temp_k1;
 ::TopLevel::x=(0);
temp_i2=(0);
temp_k2=(0);
temp_j=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_j=temp_j;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
LINK:;
if(  ::TopLevel::T_STATE_LINK1==state )
{
result= ::TopLevel::f_return;
result11=result;
result5=result11;
result6=result5;
temp_i=result6;
temp_j1=(6);
temp_k=(8);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_j=temp_j1;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK1:;
if(  ::TopLevel::T_STATE_LINK2==state )
{
result4= ::TopLevel::f_return;
result10=result4;
result8=result10;
result9=result8;
 ::TopLevel::x=result9;
temp_i1=(1);
temp_j2=(2);
temp_k1=(3);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_j=temp_j2;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK2:;
if(  ::TopLevel::T_STATE_LINK==state )
{
result2= ::TopLevel::f_return;
result3=result2;
result7=result3;
result1=result7;
cease(  ::TopLevel::x+((2)*result1) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
ENTER_f:;
if(  ::TopLevel::T_STATE_ENTER_f==state )
{
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
