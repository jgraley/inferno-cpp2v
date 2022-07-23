#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_THREAD(id_17);
}
enum id_1
{
id_10 = 2,
id_11 = 1,
id_12 = 4,
id_13 = 6,
id_14 = 9,
id_15 = 10,
id_16 = 13,
id_2 = 14,
id_3 = 12,
id_4 = 0,
id_5 = 3,
id_6 = 5,
id_7 = 7,
id_8 = 11,
id_9 = 8,
};
void id_17();
int id_21;
int id_22;
private:
unsigned int id_25;
public:
/*temp*/ int id_24;
/*temp*/ unsigned int id_23;
};
id_0 id_26("id_26");

void id_0::id_17()
{
/*temp*/ unsigned int id_18;
auto unsigned int id_19;
/*temp*/ int id_20;
do
{
if( (sc_delta_count())==(0) )
{
 ::id_0::id_21=(0);
for(  ::id_0::id_22=(0);  ::id_0::id_22<(4);  ::id_0::id_22++ )
 ::id_0::id_21+= ::id_0::id_22;
for(  ::id_0::id_22=(0);  ::id_0::id_22<=(4);  ::id_0::id_22++ )
 ::id_0::id_21+= ::id_0::id_22;
for(  ::id_0::id_22=(0); (4)!= ::id_0::id_22;  ::id_0::id_22++ )
 ::id_0::id_21+= ::id_0::id_22;
for(  ::id_0::id_22=(4);  ::id_0::id_22>(0);  ::id_0::id_22-- )
 ::id_0::id_21+= ::id_0::id_22;
for(  ::id_0::id_22=(4);  ::id_0::id_22>=(0);  ::id_0::id_22-- )
 ::id_0::id_21+= ::id_0::id_22;
 ::id_0::id_22=(0);
wait(SC_ZERO_TIME);
id_19=((!( ::id_0::id_22<(4))) ?  ::id_0::id_10 :  ::id_0::id_10);
continue;
}
if(  ::id_0::id_10==id_19 )
{
for(  ::id_0::id_22=(0);  ::id_0::id_22<(4);  ::id_0::id_22++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_22=(0);
id_19=((!( ::id_0::id_22<(4))) ?  ::id_0::id_13 :  ::id_0::id_9);
}
if(  ::id_0::id_9==id_19 )
{
 ::id_0::id_22++;
id_19=(( ::id_0::id_22<(4)) ?  ::id_0::id_9 :  ::id_0::id_13);
}
if(  ::id_0::id_13==id_19 )
{
id_19=((!( ::id_0::id_22<(4))) ?  ::id_0::id_12 :  ::id_0::id_6);
}
if(  ::id_0::id_6==id_19 )
{
 ::id_0::id_21+= ::id_0::id_22;
 ::id_0::id_22++;
id_19=(( ::id_0::id_22<(4)) ?  ::id_0::id_6 :  ::id_0::id_12);
}
if(  ::id_0::id_12==id_19 )
{
 ::id_0::id_22=(0);
id_19=((!( ::id_0::id_22< ::id_0::id_21)) ?  ::id_0::id_14 :  ::id_0::id_7);
}
if(  ::id_0::id_7==id_19 )
{
 ::id_0::id_22++;
id_19=(( ::id_0::id_22< ::id_0::id_21) ?  ::id_0::id_7 :  ::id_0::id_14);
}
if(  ::id_0::id_14==id_19 )
{
for(  ::id_0::id_22=(0);  ::id_0::id_22<(4);  ::id_0::id_22=((1)+ ::id_0::id_22) )
 ::id_0::id_21+= ::id_0::id_22;
for(  ::id_0::id_22=(0);  ::id_0::id_22<(4);  ::id_0::id_22+=(1) )
 ::id_0::id_21+= ::id_0::id_22;
for(  ::id_0::id_22=(4);  ::id_0::id_22>(0);  ::id_0::id_22=( ::id_0::id_22-(1)) )
 ::id_0::id_21+= ::id_0::id_22;
for(  ::id_0::id_22=(4);  ::id_0::id_22>(0);  ::id_0::id_22-=(1) )
 ::id_0::id_21+= ::id_0::id_22;
 ::id_0::id_22=(0);
id_19=((!( ::id_0::id_22<(4))) ?  ::id_0::id_11 :  ::id_0::id_4);
}
if(  ::id_0::id_4==id_19 )
{
 ::id_0::id_21+= ::id_0::id_22;
 ::id_0::id_22=(4);
id_19=(( ::id_0::id_22<(4)) ?  ::id_0::id_4 :  ::id_0::id_11);
}
if(  ::id_0::id_11==id_19 )
{
 ::id_0::id_22=(0);
id_19=((!( ::id_0::id_22<(4))) ?  ::id_0::id_15 :  ::id_0::id_5);
}
if(  ::id_0::id_5==id_19 )
{
 ::id_0::id_22+=(0);
 ::id_0::id_22++;
id_19=(( ::id_0::id_22<(4)) ?  ::id_0::id_5 :  ::id_0::id_15);
}
if(  ::id_0::id_15==id_19 )
{
 ::id_0::id_22=(0);
id_19=((!( ::id_0::id_22<(4))) ?  ::id_0::id_16 :  ::id_0::id_8);
}
if(  ::id_0::id_8==id_19 )
{
 ::id_0::id_23= ::id_0::id_3;
id_19= ::id_0::id_2;
}
if(  ::id_0::id_3==id_19 )
{
id_20= ::id_0::id_24;
 ::id_0::id_21+=id_20;
 ::id_0::id_22++;
id_19=(( ::id_0::id_22<(4)) ?  ::id_0::id_8 :  ::id_0::id_16);
}
if(  ::id_0::id_16==id_19 )
{
cease(  ::id_0::id_21 );
return ;
id_19= ::id_0::id_2;
}
if(  ::id_0::id_2==id_19 )
{
 ::id_0::id_25= ::id_0::id_23;
 ::id_0::id_24=(3);
id_18= ::id_0::id_25;
id_19=id_18;
}
wait(SC_ZERO_TIME);
}
while( true );
}
