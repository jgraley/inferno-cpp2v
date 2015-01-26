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
public:
void T();
private:
unsigned int link;
short j;
public:
/*temp*/ int f_i;
/*temp*/ unsigned int f_link;
/*temp*/ short f_j;
/*temp*/ unsigned int f_link1;
private:
int i;
int t;
public:
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 2U,
T_STATE_ENTER_f = 3U,
};
/*temp*/ int f_return;
/*temp*/ char f_k;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&, &&, &&, && };
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int temp_i;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_k;
/*temp*/ unsigned int temp_link;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int temp_k1;
/*temp*/ int temp_j;
/*temp*/ int temp_j1;
/*temp*/ int temp_k2;
/*temp*/ int temp_i1;
/*temp*/ int temp_j2;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int temp_i2;
/*temp*/ int result10;
/*temp*/ int result11;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_k1=(0);
temp_i=(0);
temp_j=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result6= ::TopLevel::f_return;
result1=result6;
result2=result1;
result7=result2;
temp_i2=result7;
temp_k2=(8);
temp_j2=(6);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result10= ::TopLevel::f_return;
result8=result10;
result5=result8;
result4=result5;
 ::TopLevel::x=result4;
temp_j1=(2);
temp_k=(3);
temp_i1=(1);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result3= ::TopLevel::f_return;
result11=result3;
result9=result11;
result=result9;
cease( ((2)*result)+ ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=(((3)* ::TopLevel::j)+ ::TopLevel::i);
 ::TopLevel::f_return=(((5)* ::TopLevel::k)+ ::TopLevel::t);
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
