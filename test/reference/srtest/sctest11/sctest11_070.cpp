#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_16);
}
enum id_1
{
id_10 = 5U,
id_11 = 6U,
id_12 = 10U,
id_13 = 3U,
id_14 = 11U,
id_15 = 2U,
id_2 = 13U,
id_3 = 8U,
id_4 = 1U,
id_5 = 4U,
id_6 = 9U,
id_7 = 12U,
id_8 = 0U,
id_9 = 7U,
};
void id_16();
int id_29;
int id_33;
private:
int id_34;
unsigned int id_35;
public:
/*temp*/ int id_31;
/*temp*/ int id_32;
/*temp*/ unsigned int id_30;
};
id_0 id_36("id_36");

void id_0::id_16()
{
/*temp*/ unsigned int id_17;
auto unsigned int id_18;
/*temp*/ int id_19;
/*temp*/ int id_20;
/*temp*/ int id_21;
/*temp*/ int id_22;
/*temp*/ int id_23;
/*temp*/ int id_24;
/*temp*/ bool id_25;
/*temp*/ bool id_26;
/*temp*/ bool id_27;
/*temp*/ bool id_28;
do
{
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_29=(4);
id_25=(++ ::id_0::id_29);
wait(SC_ZERO_TIME);
id_18=((!id_25) ?  ::id_0::id_15 :  ::id_0::id_8);
continue;
}
if(  ::id_0::id_8==id_18 )
{
 ::id_0::id_30= ::id_0::id_4;
 ::id_0::id_31= ::id_0::id_29;
id_18= ::id_0::id_3;
}
if(  ::id_0::id_4==id_18 )
{
id_20= ::id_0::id_32;
id_25=( ::id_0::id_33=id_20);
id_18= ::id_0::id_15;
}
if(  ::id_0::id_15==id_18 )
{
id_27=id_25;
id_27;
id_26=(!(++ ::id_0::id_29));
id_18=((!id_26) ?  ::id_0::id_13 :  ::id_0::id_10);
}
if(  ::id_0::id_13==id_18 )
{
 ::id_0::id_30= ::id_0::id_5;
 ::id_0::id_31= ::id_0::id_29;
id_18= ::id_0::id_3;
}
if(  ::id_0::id_5==id_18 )
{
id_21= ::id_0::id_32;
id_26=( ::id_0::id_33+=id_21);
id_18= ::id_0::id_10;
}
if(  ::id_0::id_10==id_18 )
{
id_28=id_26;
id_28;
 ::id_0::id_29=(0);
id_18=((!( ::id_0::id_29<(2))) ?  ::id_0::id_12 :  ::id_0::id_11);
}
if(  ::id_0::id_11==id_18 )
{
id_18=((!( ::id_0::id_29++)) ?  ::id_0::id_14 :  ::id_0::id_9);
}
if(  ::id_0::id_9==id_18 )
{
 ::id_0::id_30= ::id_0::id_6;
 ::id_0::id_31= ::id_0::id_29;
id_18= ::id_0::id_3;
}
if(  ::id_0::id_6==id_18 )
{
id_22= ::id_0::id_32;
id_19=( ::id_0::id_33+=id_22);
id_18= ::id_0::id_2;
}
if(  ::id_0::id_14==id_18 )
{
 ::id_0::id_30= ::id_0::id_7;
 ::id_0::id_31= ::id_0::id_29;
id_18= ::id_0::id_3;
}
if(  ::id_0::id_7==id_18 )
{
id_23= ::id_0::id_32;
id_19=( ::id_0::id_33-=id_23);
id_18= ::id_0::id_2;
}
if(  ::id_0::id_2==id_18 )
{
id_24=id_19;
id_24;
id_18=(( ::id_0::id_29<(2)) ?  ::id_0::id_11 :  ::id_0::id_12);
}
if(  ::id_0::id_12==id_18 )
{
cease(  ::id_0::id_33 );
return ;
id_18= ::id_0::id_3;
}
if(  ::id_0::id_3==id_18 )
{
 ::id_0::id_34= ::id_0::id_31;
 ::id_0::id_35= ::id_0::id_30;
 ::id_0::id_32=((100)/ ::id_0::id_34);
id_17= ::id_0::id_35;
id_18=id_17;
}
}
while( true );
}
