#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_16(0U)
{
SC_THREAD(id_7);
}
enum id_1
{
id_2 = 1U,
id_3 = 0U,
id_4 = 3U,
id_5 = 4U,
id_6 = 2U,
};
void id_7();
private:
int (id_17[10U]);
unsigned int (id_18[10U]);
public:
int id_13;
private:
unsigned int id_16;
public:
/*temp*/ int id_15;
/*temp*/ unsigned int id_14;
};
id_0 id_19("id_19");

void id_0::id_7()
{
/*temp*/ unsigned int id_8;
auto unsigned int id_9;
/*temp*/ int id_10;
/*temp*/ int id_11;
/*temp*/ int id_12;
do
{
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_13=(0);
id_10=(1);
 ::id_0::id_14= ::id_0::id_3;
 ::id_0::id_15=id_10;
wait(SC_ZERO_TIME);
id_9= ::id_0::id_2;
continue;
}
if(  ::id_0::id_3==id_9 )
{
cease(  ::id_0::id_13 );
return ;
id_9= ::id_0::id_2;
}
if(  ::id_0::id_2==id_9 )
{
 ::id_0::id_16++;
( ::id_0::id_17[ ::id_0::id_16])= ::id_0::id_15;
( ::id_0::id_18[ ::id_0::id_16])= ::id_0::id_14;
 ::id_0::id_13++;
id_9=((!(( ::id_0::id_17[ ::id_0::id_16])<(5))) ?  ::id_0::id_5 :  ::id_0::id_6);
}
if(  ::id_0::id_6==id_9 )
{
id_11=((1)+( ::id_0::id_17[ ::id_0::id_16]));
 ::id_0::id_14= ::id_0::id_4;
 ::id_0::id_15=id_11;
id_9= ::id_0::id_2;
}
if(  ::id_0::id_4==id_9 )
{
id_12=((1)+( ::id_0::id_17[ ::id_0::id_16]));
 ::id_0::id_14= ::id_0::id_5;
 ::id_0::id_15=id_12;
id_9= ::id_0::id_2;
}
if(  ::id_0::id_5==id_9 )
{
id_8=( ::id_0::id_18[ ::id_0::id_16]);
 ::id_0::id_16--;
id_9=id_8;
}
}
while( true );
}
