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
id_10 = 0U,
id_11 = 2U,
id_12 = 4U,
id_13 = 6U,
id_14 = 8U,
id_15 = 10U,
id_16 = 13U,
id_2 = 14U,
id_3 = 12U,
id_4 = 3U,
id_5 = 5U,
id_6 = 7U,
id_7 = 9U,
id_8 = 11U,
id_9 = 1U,
};
void id_17();
int id_23;
int id_24;
float id_28;
private:
unsigned int id_27;
public:
/*temp*/ int id_26;
/*temp*/ unsigned int id_25;
/*temp*/ unsigned int id_29;
};
id_0 id_30("id_30");

void id_0::id_17()
{
/*temp*/ unsigned int id_18;
static const unsigned int (id_19[]) = { &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20, &&id_20 };
auto unsigned int id_21;
/*temp*/ int id_22;
id_20:;
if( (sc_delta_count())==(0U) )
{
 ::id_0::id_23=(0);
for(  ::id_0::id_24=(0);  ::id_0::id_24<(4);  ::id_0::id_24++ )
 ::id_0::id_23+= ::id_0::id_24;
for(  ::id_0::id_24=(0);  ::id_0::id_24<=(4);  ::id_0::id_24++ )
 ::id_0::id_23+= ::id_0::id_24;
for(  ::id_0::id_24=(0); (4)!= ::id_0::id_24;  ::id_0::id_24++ )
 ::id_0::id_23+= ::id_0::id_24;
for(  ::id_0::id_24=(4);  ::id_0::id_24>(0);  ::id_0::id_24-- )
 ::id_0::id_23+= ::id_0::id_24;
for(  ::id_0::id_24=(4);  ::id_0::id_24>=(0);  ::id_0::id_24-- )
 ::id_0::id_23+= ::id_0::id_24;
 ::id_0::id_24=(0);
wait(SC_ZERO_TIME);
id_21=((!( ::id_0::id_24<(4))) ?  ::id_0::id_10 :  ::id_0::id_10);
goto *(id_19[id_21]);
}
if(  ::id_0::id_10==id_21 )
{
for(  ::id_0::id_24=(0);  ::id_0::id_24<(4);  ::id_0::id_24++ )
switch( 0 )
{
case 0:;
break;
}
 ::id_0::id_24=(0);
id_21=((!( ::id_0::id_24<(4))) ?  ::id_0::id_11 :  ::id_0::id_9);
}
if(  ::id_0::id_9==id_21 )
{
 ::id_0::id_24++;
id_21=(( ::id_0::id_24<(4)) ?  ::id_0::id_9 :  ::id_0::id_11);
}
if(  ::id_0::id_11==id_21 )
{
id_21=((!( ::id_0::id_24<(4))) ?  ::id_0::id_12 :  ::id_0::id_4);
}
if(  ::id_0::id_4==id_21 )
{
 ::id_0::id_23+= ::id_0::id_24;
 ::id_0::id_24++;
id_21=(( ::id_0::id_24<(4)) ?  ::id_0::id_4 :  ::id_0::id_12);
}
if(  ::id_0::id_12==id_21 )
{
 ::id_0::id_24=(0);
id_21=((!( ::id_0::id_24< ::id_0::id_23)) ?  ::id_0::id_13 :  ::id_0::id_5);
}
if(  ::id_0::id_5==id_21 )
{
 ::id_0::id_24++;
id_21=(( ::id_0::id_24< ::id_0::id_23) ?  ::id_0::id_5 :  ::id_0::id_13);
}
if(  ::id_0::id_13==id_21 )
{
for(  ::id_0::id_24=(0);  ::id_0::id_24<(4);  ::id_0::id_24=((1)+ ::id_0::id_24) )
 ::id_0::id_23+= ::id_0::id_24;
for(  ::id_0::id_24=(0);  ::id_0::id_24<(4);  ::id_0::id_24+=(1) )
 ::id_0::id_23+= ::id_0::id_24;
for(  ::id_0::id_24=(4);  ::id_0::id_24>(0);  ::id_0::id_24=( ::id_0::id_24-(1)) )
 ::id_0::id_23+= ::id_0::id_24;
for(  ::id_0::id_24=(4);  ::id_0::id_24>(0);  ::id_0::id_24-=(1) )
 ::id_0::id_23+= ::id_0::id_24;
 ::id_0::id_24=(0);
id_21=((!( ::id_0::id_24<(4))) ?  ::id_0::id_14 :  ::id_0::id_6);
}
if(  ::id_0::id_6==id_21 )
{
 ::id_0::id_23+= ::id_0::id_24;
 ::id_0::id_24=(4);
id_21=(( ::id_0::id_24<(4)) ?  ::id_0::id_6 :  ::id_0::id_14);
}
if(  ::id_0::id_14==id_21 )
{
 ::id_0::id_24=(0);
id_21=((!( ::id_0::id_24<(4))) ?  ::id_0::id_15 :  ::id_0::id_7);
}
if(  ::id_0::id_7==id_21 )
{
 ::id_0::id_24+=(0);
 ::id_0::id_24++;
id_21=(( ::id_0::id_24<(4)) ?  ::id_0::id_7 :  ::id_0::id_15);
}
if(  ::id_0::id_15==id_21 )
{
 ::id_0::id_24=(0);
id_21=((!( ::id_0::id_24<(4))) ?  ::id_0::id_16 :  ::id_0::id_8);
}
if(  ::id_0::id_8==id_21 )
{
 ::id_0::id_25= ::id_0::id_3;
id_21= ::id_0::id_2;
}
if(  ::id_0::id_3==id_21 )
{
id_22= ::id_0::id_26;
 ::id_0::id_23+=id_22;
 ::id_0::id_24++;
id_21=(( ::id_0::id_24<(4)) ?  ::id_0::id_8 :  ::id_0::id_16);
}
if(  ::id_0::id_16==id_21 )
{
cease(  ::id_0::id_23 );
return ;
id_21= ::id_0::id_2;
}
if(  ::id_0::id_2==id_21 )
{
 ::id_0::id_27= ::id_0::id_25;
 ::id_0::id_26=(3);
id_18= ::id_0::id_27;
id_21=id_18;
}
goto *(id_19[id_21]);
}
