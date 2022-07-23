#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_18);
}
/*temp*/ unsigned int id_1;
enum id_2
{
id_10 = 8,
id_11 = 2,
id_12 = 1,
id_13 = 4,
id_14 = 6,
id_15 = 9,
id_16 = 10,
id_17 = 13,
id_3 = 14,
id_4 = 12,
id_5 = 0,
id_6 = 3,
id_7 = 5,
id_8 = 7,
id_9 = 11,
};
void id_18();
int id_19;
int id_20;
private:
unsigned int id_21;
unsigned int id_25;
public:
/*temp*/ int id_23;
/*temp*/ int id_24;
/*temp*/ unsigned int id_22;
};
id_0 id_26("id_26");

void id_0::id_18()
{
if( (sc_delta_count())==(0) )
{
 ::id_0::id_19=(0);
for(  ::id_0::id_20=(0);  ::id_0::id_20<(4);  ::id_0::id_20++ )
 ::id_0::id_19+= ::id_0::id_20;
for(  ::id_0::id_20=(0);  ::id_0::id_20<=(4);  ::id_0::id_20++ )
 ::id_0::id_19+= ::id_0::id_20;
for(  ::id_0::id_20=(0); (4)!= ::id_0::id_20;  ::id_0::id_20++ )
 ::id_0::id_19+= ::id_0::id_20;
for(  ::id_0::id_20=(4);  ::id_0::id_20>(0);  ::id_0::id_20-- )
 ::id_0::id_19+= ::id_0::id_20;
for(  ::id_0::id_20=(4);  ::id_0::id_20>=(0);  ::id_0::id_20-- )
 ::id_0::id_19+= ::id_0::id_20;
 ::id_0::id_20=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_21=((!( ::id_0::id_20<(4))) ?  ::id_0::id_11 :  ::id_0::id_11);
return ;
}
if(  ::id_0::id_11== ::id_0::id_21 )
{
for(  ::id_0::id_20=(0);  ::id_0::id_20<(4);  ::id_0::id_20++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_20=(0);
 ::id_0::id_21=((!( ::id_0::id_20<(4))) ?  ::id_0::id_14 :  ::id_0::id_10);
}
if(  ::id_0::id_10== ::id_0::id_21 )
{
 ::id_0::id_20++;
 ::id_0::id_21=(( ::id_0::id_20<(4)) ?  ::id_0::id_10 :  ::id_0::id_14);
}
if(  ::id_0::id_14== ::id_0::id_21 )
{
 ::id_0::id_21=((!( ::id_0::id_20<(4))) ?  ::id_0::id_13 :  ::id_0::id_7);
}
if(  ::id_0::id_7== ::id_0::id_21 )
{
 ::id_0::id_19+= ::id_0::id_20;
 ::id_0::id_20++;
 ::id_0::id_21=(( ::id_0::id_20<(4)) ?  ::id_0::id_7 :  ::id_0::id_13);
}
if(  ::id_0::id_13== ::id_0::id_21 )
{
 ::id_0::id_20=(0);
 ::id_0::id_21=((!( ::id_0::id_20< ::id_0::id_19)) ?  ::id_0::id_15 :  ::id_0::id_8);
}
if(  ::id_0::id_8== ::id_0::id_21 )
{
 ::id_0::id_20++;
 ::id_0::id_21=(( ::id_0::id_20< ::id_0::id_19) ?  ::id_0::id_8 :  ::id_0::id_15);
}
if(  ::id_0::id_15== ::id_0::id_21 )
{
for(  ::id_0::id_20=(0);  ::id_0::id_20<(4);  ::id_0::id_20=((1)+ ::id_0::id_20) )
 ::id_0::id_19+= ::id_0::id_20;
for(  ::id_0::id_20=(0);  ::id_0::id_20<(4);  ::id_0::id_20+=(1) )
 ::id_0::id_19+= ::id_0::id_20;
for(  ::id_0::id_20=(4);  ::id_0::id_20>(0);  ::id_0::id_20=( ::id_0::id_20-(1)) )
 ::id_0::id_19+= ::id_0::id_20;
for(  ::id_0::id_20=(4);  ::id_0::id_20>(0);  ::id_0::id_20-=(1) )
 ::id_0::id_19+= ::id_0::id_20;
 ::id_0::id_20=(0);
 ::id_0::id_21=((!( ::id_0::id_20<(4))) ?  ::id_0::id_12 :  ::id_0::id_5);
}
if(  ::id_0::id_5== ::id_0::id_21 )
{
 ::id_0::id_19+= ::id_0::id_20;
 ::id_0::id_20=(4);
 ::id_0::id_21=(( ::id_0::id_20<(4)) ?  ::id_0::id_5 :  ::id_0::id_12);
}
if(  ::id_0::id_12== ::id_0::id_21 )
{
 ::id_0::id_20=(0);
 ::id_0::id_21=((!( ::id_0::id_20<(4))) ?  ::id_0::id_16 :  ::id_0::id_6);
}
if(  ::id_0::id_6== ::id_0::id_21 )
{
 ::id_0::id_20+=(0);
 ::id_0::id_20++;
 ::id_0::id_21=(( ::id_0::id_20<(4)) ?  ::id_0::id_6 :  ::id_0::id_16);
}
if(  ::id_0::id_16== ::id_0::id_21 )
{
 ::id_0::id_20=(0);
 ::id_0::id_21=((!( ::id_0::id_20<(4))) ?  ::id_0::id_17 :  ::id_0::id_9);
}
if(  ::id_0::id_9== ::id_0::id_21 )
{
 ::id_0::id_22= ::id_0::id_4;
 ::id_0::id_21= ::id_0::id_3;
}
if(  ::id_0::id_4== ::id_0::id_21 )
{
 ::id_0::id_23= ::id_0::id_24;
 ::id_0::id_19+= ::id_0::id_23;
 ::id_0::id_20++;
 ::id_0::id_21=(( ::id_0::id_20<(4)) ?  ::id_0::id_9 :  ::id_0::id_17);
}
if(  ::id_0::id_17== ::id_0::id_21 )
{
cease(  ::id_0::id_19 );
return ;
 ::id_0::id_21= ::id_0::id_3;
}
if(  ::id_0::id_3== ::id_0::id_21 )
{
 ::id_0::id_25= ::id_0::id_22;
 ::id_0::id_24=(3);
 ::id_0::id_1= ::id_0::id_25;
 ::id_0::id_21= ::id_0::id_1;
}
next_trigger(SC_ZERO_TIME);
}
