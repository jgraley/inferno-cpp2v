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
/*temp*/ short f_j;
private:
unsigned int link;
public:
int x;
/*temp*/ int f_i;
private:
short j;
int t;
public:
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link1;
enum TStates
{
T_STATE_ENTER_f = 3U,
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 2U,
};
/*temp*/ int f_return;
/*temp*/ char f_k;
private:
int i;
char k;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ unsigned int temp_link;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int temp_k;
/*temp*/ int temp_i;
/*temp*/ int result4;
/*temp*/ int temp_k1;
/*temp*/ int temp_j;
/*temp*/ int result5;
/*temp*/ int result6;
auto unsigned int state;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int temp_k2;
/*temp*/ int temp_j1;
/*temp*/ int temp_i1;
/*temp*/ int temp_j2;
/*temp*/ int temp_i2;
/*temp*/ int result11;
static const unsigned int (lmap[]) = { &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK };
 ::TopLevel::x=(0);
temp_k=(0);
temp_i=(0);
temp_j1=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
ENTER_f_LINK_LINK_LINK:;
if( state== ::TopLevel::T_STATE_LINK1 )
{
result= ::TopLevel::f_return;
result3=result;
result7=result3;
result1=result7;
temp_i2=result1;
temp_k1=(8);
temp_j=(6);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result5= ::TopLevel::f_return;
result6=result5;
result10=result6;
result11=result10;
 ::TopLevel::x=result11;
temp_j2=(2);
temp_k2=(3);
temp_i1=(1);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result9= ::TopLevel::f_return;
result4=result9;
result8=result4;
result2=result8;
cease(  ::TopLevel::x+(result2*(2)) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
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
