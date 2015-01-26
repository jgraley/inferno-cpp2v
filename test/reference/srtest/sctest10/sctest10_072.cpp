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
/*temp*/ char f_k;
private:
int i;
public:
/*temp*/ int f_return;
int x;
private:
unsigned int state;
public:
/*temp*/ short f_j;
private:
unsigned int link;
public:
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK2 = 2U,
};
/*temp*/ int f_i;
private:
int t;
short j;
char k;
public:
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_j;
/*temp*/ int temp_k;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int temp_j1;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_k1;
/*temp*/ int temp_i;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int temp_i1;
/*temp*/ int result9;
/*temp*/ int temp_j2;
/*temp*/ int result10;
/*temp*/ int temp_k2;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_i2;
/*temp*/ int result11;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
temp_k1=(8);
temp_j1=(6);
temp_j=(0);
temp_i2=(0);
temp_k=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
result2= ::TopLevel::f_return;
result7=result2;
result10=result7;
result1=result10;
temp_i=result1;
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
result3= ::TopLevel::f_return;
result5=result3;
result11=result5;
result6=result11;
 ::TopLevel::x=result6;
temp_i1=(1);
temp_k2=(3);
temp_j2=(2);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK2 )
{
result= ::TopLevel::f_return;
result4=result;
result8=result4;
result9=result8;
cease(  ::TopLevel::x+(result9*(2)) );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
temp_link= ::TopLevel::link;
 ::TopLevel::state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
