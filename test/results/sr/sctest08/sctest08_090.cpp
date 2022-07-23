#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 ) :
id_21(0)
{
SC_THREAD(id_12);
}
enum id_1
{
id_10 = 6,
id_11 = 8,
id_2 = 3,
id_3 = 9,
id_4 = 2,
id_5 = 5,
id_6 = 0,
id_7 = 4,
id_8 = 1,
id_9 = 7,
};
void id_12();
private:
int (id_23[10]);
unsigned int (id_22[10]);
unsigned int id_18;
unsigned int id_21;
unsigned int id_26;
public:
/*temp*/ int id_19;
/*temp*/ unsigned int id_20;
/*temp*/ unsigned int id_25;
};
id_0 id_27("id_27");
int id_16;
int id_17;
int id_24;

void id_0::id_12()
{
/*temp*/ unsigned int id_13;
/*temp*/ unsigned int id_14;
/*temp*/ int id_15;
do
{
if( (sc_delta_count())==(0) )
{
 ::id_16=(1);
 ::id_17=(0);
wait(SC_ZERO_TIME);
 ::id_0::id_18=((!( ::id_17<(4))) ?  ::id_0::id_9 :  ::id_0::id_6);
continue;
}
if(  ::id_0::id_6== ::id_0::id_18 )
{
 ::id_16=( ::id_16+ ::id_17);
id_15=(3);
 ::id_0::id_19=id_15;
 ::id_0::id_20= ::id_0::id_5;
 ::id_0::id_18= ::id_0::id_2;
}
if(  ::id_0::id_5== ::id_0::id_18 )
{
 ::id_16=((2)* ::id_16);
wait(SC_ZERO_TIME);
 ::id_0::id_18= ::id_0::id_10;
continue;
}
if(  ::id_0::id_10== ::id_0::id_18 )
{
 ::id_17=((1)+ ::id_17);
 ::id_0::id_18=(( ::id_17<(4)) ?  ::id_0::id_6 :  ::id_0::id_9);
}
if(  ::id_0::id_9== ::id_0::id_18 )
{
cease(  ::id_16 );
return ;
 ::id_0::id_18= ::id_0::id_2;
}
if(  ::id_0::id_2== ::id_0::id_18 )
{
 ::id_0::id_21++;
( ::id_0::id_22[ ::id_0::id_21])= ::id_0::id_20;
( ::id_0::id_23[ ::id_0::id_21])= ::id_0::id_19;
 ::id_24=(0);
 ::id_0::id_18=((!( ::id_24<( ::id_0::id_23[ ::id_0::id_21]))) ?  ::id_0::id_8 :  ::id_0::id_7);
}
if(  ::id_0::id_7== ::id_0::id_18 )
{
wait(SC_ZERO_TIME);
 ::id_0::id_18= ::id_0::id_11;
continue;
}
if(  ::id_0::id_11== ::id_0::id_18 )
{
 ::id_16=((1)+ ::id_16);
 ::id_24=((1)+ ::id_24);
 ::id_0::id_18=(( ::id_24<( ::id_0::id_23[ ::id_0::id_21])) ?  ::id_0::id_7 :  ::id_0::id_8);
}
if(  ::id_0::id_8== ::id_0::id_18 )
{
 ::id_0::id_25= ::id_0::id_4;
 ::id_0::id_18= ::id_0::id_3;
}
if(  ::id_0::id_4== ::id_0::id_18 )
{
id_13=( ::id_0::id_22[ ::id_0::id_21]);
 ::id_0::id_21--;
 ::id_0::id_18=id_13;
}
if(  ::id_0::id_3== ::id_0::id_18 )
{
 ::id_0::id_26= ::id_0::id_25;
 ::id_16=( ::id_16-(1));
id_14= ::id_0::id_26;
 ::id_0::id_18=id_14;
}
wait(SC_ZERO_TIME);
}
while( true );
}
