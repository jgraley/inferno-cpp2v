#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_21(0U)
{
SC_THREAD(id_12);
}
enum id_1
{
id_10 = 2U,
id_11 = 6U,
id_2 = 4U,
id_3 = 9U,
id_4 = 1U,
id_5 = 8U,
id_6 = 0U,
id_7 = 5U,
id_8 = 3U,
id_9 = 7U,
};
void id_12();
private:
int (id_23[10U]);
unsigned int (id_22[10U]);
unsigned int id_21;
unsigned int id_26;
public:
/*temp*/ int id_19;
/*temp*/ unsigned int id_20;
/*temp*/ unsigned int id_25;
};
id_0 id_27("id_27");
int id_17;
int id_18;
int id_24;

void id_0::id_12()
{
/*temp*/ unsigned int id_13;
/*temp*/ unsigned int id_14;
auto unsigned int id_15;
/*temp*/ int id_16;
do
{
if( (sc_delta_count())==(0U) )
{
 ::id_17=(1);
 ::id_18=(0);
wait(SC_ZERO_TIME);
id_15=((!( ::id_18<(4))) ?  ::id_0::id_8 :  ::id_0::id_6);
continue;
}
if(  ::id_0::id_6==id_15 )
{
 ::id_17=( ::id_17+ ::id_18);
id_16=(3);
 ::id_0::id_19=id_16;
 ::id_0::id_20= ::id_0::id_4;
id_15= ::id_0::id_2;
}
if(  ::id_0::id_4==id_15 )
{
 ::id_17=((2)* ::id_17);
wait(SC_ZERO_TIME);
id_15= ::id_0::id_10;
continue;
}
if(  ::id_0::id_10==id_15 )
{
 ::id_18=((1)+ ::id_18);
id_15=(( ::id_18<(4)) ?  ::id_0::id_6 :  ::id_0::id_8);
}
if(  ::id_0::id_8==id_15 )
{
cease(  ::id_17 );
return ;
id_15= ::id_0::id_2;
}
if(  ::id_0::id_2==id_15 )
{
 ::id_0::id_21++;
( ::id_0::id_22[ ::id_0::id_21])= ::id_0::id_20;
( ::id_0::id_23[ ::id_0::id_21])= ::id_0::id_19;
 ::id_24=(0);
id_15=((!( ::id_24<( ::id_0::id_23[ ::id_0::id_21]))) ?  ::id_0::id_9 :  ::id_0::id_7);
}
if(  ::id_0::id_7==id_15 )
{
wait(SC_ZERO_TIME);
id_15= ::id_0::id_11;
continue;
}
if(  ::id_0::id_11==id_15 )
{
 ::id_17=((1)+ ::id_17);
 ::id_24=((1)+ ::id_24);
id_15=(( ::id_24<( ::id_0::id_23[ ::id_0::id_21])) ?  ::id_0::id_7 :  ::id_0::id_9);
}
if(  ::id_0::id_9==id_15 )
{
 ::id_0::id_25= ::id_0::id_5;
id_15= ::id_0::id_3;
}
if(  ::id_0::id_5==id_15 )
{
id_13=( ::id_0::id_22[ ::id_0::id_21]);
 ::id_0::id_21--;
id_15=id_13;
}
if(  ::id_0::id_3==id_15 )
{
 ::id_0::id_26= ::id_0::id_25;
 ::id_17=( ::id_17-(1));
id_14= ::id_0::id_26;
id_15=id_14;
}
}
while( true );
}
