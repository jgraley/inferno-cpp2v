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
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 1U,
T_STATE_LINK = 2U,
T_STATE_ENTER_f = 3U,
};
int x;
void T();
private:
int i;
short j;
char k;
int t;
unsigned int link;
public:
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ unsigned int f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&
#error identifier ENTER_f_LINK_LINK_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_LINK_LINK_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_LINK_LINK_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_LINK_LINK_LINK undeclared not supported in RenderIdentifier
 };
auto unsigned int state;
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ int temp_k2;
/*temp*/ int temp_j1;
/*temp*/ int temp_j;
/*temp*/ int temp_k;
/*temp*/ int temp_i2;
/*temp*/ int temp_j2;
/*temp*/ int temp_k1;
/*temp*/ int result7;
/*temp*/ int result3;
/*temp*/ int result;
/*temp*/ int result2;
/*temp*/ int result11;
/*temp*/ int result9;
/*temp*/ int result8;
/*temp*/ int result1;
/*temp*/ int result10;
/*temp*/ int result6;
/*temp*/ int result4;
/*temp*/ int result5;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_i1=(0);
temp_k2=(0);
temp_j1=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result2= ::TopLevel::f_return;
result11=result2;
result=result11;
result3=result;
temp_i=result3;
temp_j=(6);
temp_k=(8);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result9= ::TopLevel::f_return;
result1=result9;
result8=result1;
result7=result8;
 ::TopLevel::x=result7;
temp_i2=(1);
temp_j2=(2);
temp_k1=(3);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result5= ::TopLevel::f_return;
result4=result5;
result6=result4;
result10=result6;
cease(  ::TopLevel::x+(result10*(2)) );
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
