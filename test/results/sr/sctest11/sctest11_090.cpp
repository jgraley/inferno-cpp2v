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
id_10 = 2,
id_11 = 5,
id_12 = 12,
id_13 = 6,
id_14 = 10,
id_15 = 4,
id_2 = 1,
id_3 = 13,
id_4 = 3,
id_5 = 7,
id_6 = 9,
id_7 = 11,
id_8 = 0,
id_9 = 8,
};
void id_16();
int id_28;
int id_33;
private:
int id_35;
unsigned int id_29;
unsigned int id_34;
public:
/*temp*/ int id_30;
/*temp*/ int id_32;
/*temp*/ unsigned int id_31;
};
id_0 id_36("id_36");

void id_0::id_16()
{
/*temp*/ unsigned int id_17;
/*temp*/ int id_18;
/*temp*/ int id_19;
/*temp*/ int id_20;
/*temp*/ int id_21;
/*temp*/ int id_22;
/*temp*/ int id_23;
/*temp*/ bool id_24;
/*temp*/ bool id_25;
/*temp*/ bool id_26;
/*temp*/ bool id_27;
do
{
if( (sc_delta_count())==(0) )
{
 ::id_0::id_28=(4);
id_24=(++ ::id_0::id_28);
wait(SC_ZERO_TIME);
 ::id_0::id_29=((!id_24) ?  ::id_0::id_15 :  ::id_0::id_8);
continue;
}
if(  ::id_0::id_8== ::id_0::id_29 )
{
 ::id_0::id_30= ::id_0::id_28;
 ::id_0::id_31= ::id_0::id_4;
 ::id_0::id_29= ::id_0::id_3;
}
if(  ::id_0::id_4== ::id_0::id_29 )
{
id_19= ::id_0::id_32;
id_24=( ::id_0::id_33=id_19);
 ::id_0::id_29= ::id_0::id_15;
}
if(  ::id_0::id_15== ::id_0::id_29 )
{
id_26=id_24;
id_26;
id_25=(!(++ ::id_0::id_28));
 ::id_0::id_29=((!id_25) ?  ::id_0::id_13 :  ::id_0::id_10);
}
if(  ::id_0::id_13== ::id_0::id_29 )
{
 ::id_0::id_30= ::id_0::id_28;
 ::id_0::id_31= ::id_0::id_5;
 ::id_0::id_29= ::id_0::id_3;
}
if(  ::id_0::id_5== ::id_0::id_29 )
{
id_20= ::id_0::id_32;
id_25=( ::id_0::id_33+=id_20);
 ::id_0::id_29= ::id_0::id_10;
}
if(  ::id_0::id_10== ::id_0::id_29 )
{
id_27=id_25;
id_27;
 ::id_0::id_28=(0);
 ::id_0::id_29=((!( ::id_0::id_28<(2))) ?  ::id_0::id_12 :  ::id_0::id_11);
}
if(  ::id_0::id_11== ::id_0::id_29 )
{
 ::id_0::id_29=((!( ::id_0::id_28++)) ?  ::id_0::id_14 :  ::id_0::id_9);
}
if(  ::id_0::id_9== ::id_0::id_29 )
{
 ::id_0::id_30= ::id_0::id_28;
 ::id_0::id_31= ::id_0::id_6;
 ::id_0::id_29= ::id_0::id_3;
}
if(  ::id_0::id_6== ::id_0::id_29 )
{
id_21= ::id_0::id_32;
id_18=( ::id_0::id_33+=id_21);
 ::id_0::id_29= ::id_0::id_2;
}
if(  ::id_0::id_14== ::id_0::id_29 )
{
 ::id_0::id_30= ::id_0::id_28;
 ::id_0::id_31= ::id_0::id_7;
 ::id_0::id_29= ::id_0::id_3;
}
if(  ::id_0::id_7== ::id_0::id_29 )
{
id_22= ::id_0::id_32;
id_18=( ::id_0::id_33-=id_22);
 ::id_0::id_29= ::id_0::id_2;
}
if(  ::id_0::id_2== ::id_0::id_29 )
{
id_23=id_18;
id_23;
 ::id_0::id_29=(( ::id_0::id_28<(2)) ?  ::id_0::id_11 :  ::id_0::id_12);
}
if(  ::id_0::id_12== ::id_0::id_29 )
{
cease(  ::id_0::id_33 );
return ;
 ::id_0::id_29= ::id_0::id_3;
}
if(  ::id_0::id_3== ::id_0::id_29 )
{
 ::id_0::id_34= ::id_0::id_31;
 ::id_0::id_35= ::id_0::id_30;
 ::id_0::id_32=((100)/ ::id_0::id_35);
id_17= ::id_0::id_34;
 ::id_0::id_29=id_17;
}
wait(SC_ZERO_TIME);
}
while( true );
}
