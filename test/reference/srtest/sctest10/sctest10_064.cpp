#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ char f_k;
private:
unsigned int link;
public:
/*temp*/ int f_return;
private:
char k;
public:
void T();
private:
short j;
public:
int x;
private:
int i;
public:
/*temp*/ unsigned int f_link;
/*temp*/ short f_j;
/*temp*/ unsigned int f_link1;
/*temp*/ int f_i;
enum TStates
{
T_STATE_LINK = 2U,
T_STATE_LINK1 = 1U,
T_STATE_LINK2 = 0U,
T_STATE_ENTER_f = 3U,
};
private:
int t;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
static const unsigned int (lmap[]) = { &&LINK2, &&LINK1, &&LINK, &&ENTER_f };
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int temp_j;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ int result4;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i2;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int temp_k;
/*temp*/ int result8;
/*temp*/ int temp_k1;
/*temp*/ int temp_k2;
/*temp*/ int result9;
/*temp*/ int temp_j1;
/*temp*/ int temp_j2;
auto unsigned int state;
/*temp*/ int result10;
/*temp*/ int result11;
 ::TopLevel::x=(0);
temp_k1=(8);
temp_j=(6);
temp_j1=(0);
temp_i2=(0);
temp_k=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
ENTER_f:;
LINK:;
LINK1:;
LINK2:;
if( state== ::TopLevel::T_STATE_LINK2 )
{
result2= ::TopLevel::f_return;
result7=result2;
result3=result7;
result8=result3;
temp_i=result8;
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK1==state )
{
result6= ::TopLevel::f_return;
result9=result6;
result11=result9;
result4=result11;
 ::TopLevel::x=result4;
temp_i1=(1);
temp_k2=(3);
temp_j2=(2);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result1= ::TopLevel::f_return;
result=result1;
result10=result;
result5=result10;
cease( ((2)*result5)+ ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
