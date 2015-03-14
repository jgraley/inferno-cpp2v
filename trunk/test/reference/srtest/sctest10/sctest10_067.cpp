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
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ unsigned int f_link1;
private:
int t;
int i;
short j;
char k;
unsigned int link;
public:
enum TStates
{
T_STATE_LINK = 0U,
T_STATE_LINK1 = 1U,
T_STATE_LINK2 = 2U,
T_STATE_ENTER_f = 3U,
};
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto unsigned int state;
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
/*temp*/ int temp_k;
/*temp*/ int result1;
/*temp*/ int temp_j;
/*temp*/ int result2;
/*temp*/ int temp_i;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_k1;
/*temp*/ int result5;
/*temp*/ int temp_j1;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int temp_k2;
/*temp*/ int result8;
/*temp*/ int temp_j2;
/*temp*/ int result9;
/*temp*/ int temp_i1;
/*temp*/ int result10;
/*temp*/ int temp_i2;
/*temp*/ int result11;
static const unsigned int (lmap[]) = { &&
#error identifier ENTER_f_LINK_LINK_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_LINK_LINK_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_LINK_LINK_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_LINK_LINK_LINK undeclared not supported in RenderIdentifier
 };
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_i1=(0);
temp_j2=(0);
temp_k2=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result7= ::TopLevel::f_return;
result8=result7;
result9=result8;
result10=result9;
temp_i2=result10;
temp_j1=(6);
temp_k1=(8);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result4= ::TopLevel::f_return;
result5=result4;
result6=result5;
result11=result6;
 ::TopLevel::x=result11;
temp_i=(1);
temp_j=(2);
temp_k=(3);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result= ::TopLevel::f_return;
result1=result;
result2=result1;
result3=result2;
cease(  ::TopLevel::x+(result3*(2)) );
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
}
while( true );
}
