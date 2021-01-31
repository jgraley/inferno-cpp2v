#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_18(0U)
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
int (id_19[10U]);
unsigned int (id_20[10U]);
public:
int id_15;
private:
unsigned int id_18;
public:
/*temp*/ int id_17;
/*temp*/ unsigned int id_16;
/*temp*/ unsigned int id_21;
};
id_0 id_22("id_22");

void id_0::id_7()
{
/*temp*/ unsigned int id_8;
static const unsigned int (id_9[]) = { &&id_10, &&id_10, &&id_10, &&id_10, &&id_10 };
auto unsigned int id_11;
/*temp*/ int id_12;
/*temp*/ int id_13;
/*temp*/ int id_14;
do
{
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_15=(0);
id_12=(1);
 ::id_0::id_16= ::id_0::id_3;
 ::id_0::id_17=id_12;
wait(SC_ZERO_TIME);
id_11= ::id_0::id_2;
continue;
}
if(  ::id_0::id_3==id_11 )
{
cease(  ::id_0::id_15 );
return ;
id_11= ::id_0::id_2;
}
if(  ::id_0::id_2==id_11 )
{
 ::id_0::id_18++;
( ::id_0::id_19[ ::id_0::id_18])= ::id_0::id_17;
( ::id_0::id_20[ ::id_0::id_18])= ::id_0::id_16;
 ::id_0::id_15++;
id_11=((!(( ::id_0::id_19[ ::id_0::id_18])<(5))) ?  ::id_0::id_5 :  ::id_0::id_6);
}
if(  ::id_0::id_6==id_11 )
{
id_13=((1)+( ::id_0::id_19[ ::id_0::id_18]));
 ::id_0::id_16= ::id_0::id_4;
 ::id_0::id_17=id_13;
id_11= ::id_0::id_2;
}
if(  ::id_0::id_4==id_11 )
{
id_14=((1)+( ::id_0::id_19[ ::id_0::id_18]));
 ::id_0::id_16= ::id_0::id_5;
 ::id_0::id_17=id_14;
id_11= ::id_0::id_2;
}
if(  ::id_0::id_5==id_11 )
{
id_8=( ::id_0::id_20[ ::id_0::id_18]);
 ::id_0::id_18--;
id_11=id_8;
}
}
while( true );
}
