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
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 2U,
T_STATE_ENTER_f = 3U,
};
/*temp*/ unsigned int f_link;
private:
unsigned int state;
public:
/*temp*/ int f_i;
private:
char k;
public:
/*temp*/ int f_return;
/*temp*/ short f_j;
private:
int t;
short j;
public:
/*temp*/ char f_k;
void T();
private:
unsigned int link;
public:
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int temp_j;
/*temp*/ int temp_k;
/*temp*/ int temp_j1;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int temp_i;
/*temp*/ int result4;
/*temp*/ int temp_j2;
/*temp*/ int temp_k1;
/*temp*/ int temp_i1;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i2;
/*temp*/ int temp_k2;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int result11;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_k1=(0);
temp_i1=(0);
temp_j1=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
result5= ::TopLevel::f_return;
result1=result5;
result3=result1;
result4=result3;
temp_i=result4;
temp_k=(8);
temp_j=(6);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
result= ::TopLevel::f_return;
result11=result;
result10=result11;
result7=result10;
 ::TopLevel::x=result7;
temp_j2=(2);
temp_k2=(3);
temp_i2=(1);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK2 )
{
result6= ::TopLevel::f_return;
result2=result6;
result9=result2;
result8=result9;
cease(  ::TopLevel::x+((2)*result8) );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=(((3)* ::TopLevel::j)+ ::TopLevel::i);
 ::TopLevel::f_return=(((5)* ::TopLevel::k)+ ::TopLevel::t);
temp_link= ::TopLevel::link;
 ::TopLevel::state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
