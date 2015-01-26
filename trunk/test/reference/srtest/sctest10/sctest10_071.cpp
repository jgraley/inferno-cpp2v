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
public:
/*temp*/ int f_i;
private:
int t;
public:
int x;
private:
char k;
public:
enum TStates
{
T_STATE_LINK = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK1 = 1U,
T_STATE_LINK2 = 0U,
};
private:
short j;
public:
/*temp*/ unsigned int f_link;
/*temp*/ char f_k;
/*temp*/ int f_return;
/*temp*/ short f_j;
void T();
private:
unsigned int link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int temp_j;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ int temp_i;
/*temp*/ int temp_j1;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int temp_k;
/*temp*/ int temp_i1;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int temp_j2;
auto unsigned int state;
/*temp*/ int result10;
/*temp*/ int temp_i2;
/*temp*/ unsigned int temp_link;
/*temp*/ int result11;
/*temp*/ int temp_k1;
/*temp*/ int temp_k2;
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
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result11= ::TopLevel::f_return;
result5=result11;
result=result5;
result2=result;
temp_i2=result2;
temp_k2=(8);
temp_j1=(6);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result8= ::TopLevel::f_return;
result7=result8;
result4=result7;
result9=result4;
 ::TopLevel::x=result9;
temp_j2=(2);
temp_k=(3);
temp_i1=(1);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result1= ::TopLevel::f_return;
result6=result1;
result10=result6;
result3=result10;
cease( ((2)*result3)+ ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=(((3)* ::TopLevel::j)+ ::TopLevel::i);
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
temp_link= ::TopLevel::link;
state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
